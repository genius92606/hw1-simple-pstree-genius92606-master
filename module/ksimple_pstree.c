#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>
#include <linux/list.h>

#define NETLINK_USER 31

struct sock *nl_sk = NULL;

//after recieve option, do recursive for option c
static void C_function(int hw_pid, char *trns_msg,int space)
{

    char int_to_str [10];
    int loop=1;

    struct task_struct *ts;
    struct pid *p;


    p = find_get_pid(hw_pid);
    if(p==NULL) {
        if(space=00)
            strcpy(trns_msg,"\n");
        return;
    }


    ts=pid_task(p,PIDTYPE_PID);
    struct task_struct *list_children;
    struct list_head *pos_children=&ts->children;
    if(space==0) {
        for(loop=0; loop<space; loop++)
            strcat(trns_msg," ");
        strcat(trns_msg,ts->comm);
        strcat(trns_msg,"(");
        sprintf(int_to_str,"%d",ts->pid);
        strcat(trns_msg,int_to_str);
        strcat(trns_msg,")");
        strcat(trns_msg,"\n");
    }

    space=space+4;
    list_for_each(pos_children,&ts->children) {
        list_children=list_entry(pos_children,struct task_struct,sibling);
        for(loop=0; loop<space; loop++)
            strcat(trns_msg," ");
        strcat(trns_msg, list_children->comm);
        strcat(trns_msg,"(");
        sprintf(int_to_str,"%d",list_children->pid);
        strcat(trns_msg, int_to_str);
        strcat(trns_msg,")");
        strcat(trns_msg,"\n");
        C_function(list_children->pid,trns_msg,space);

    }
}
//process option s
static void S_function(int hw_pid, char *trns_msg)
{

    char int_to_str [10];

    struct task_struct *ts;
    struct pid *p;


    p = find_get_pid(hw_pid);
    if(p==NULL)
        return;
    ts=pid_task(p,PIDTYPE_PID);

    strcat(trns_msg,ts->comm);
    strcat(trns_msg,"(");
    sprintf(int_to_str,"%d",ts->pid);
    strcat(trns_msg,int_to_str);
    strcat(trns_msg,")");
    strcat(trns_msg,"\n");
    struct task_struct *list_sibling;
    struct list_head *pos_sibling=&ts->sibling;
    list_for_each(pos_sibling,&ts->sibling) {
        list_sibling=list_entry(pos_sibling,struct task_struct,sibling);
        strcat(trns_msg, list_sibling->comm);
        strcat(trns_msg,"(");
        sprintf(int_to_str,"%d",list_sibling->pid);
        strcat(trns_msg, int_to_str);
        strcat(trns_msg,")");
        strcat(trns_msg,"\n");
    }
}
static int P_long(int hw_pid,int i)
{
    struct task_struct *ts;
    struct pid *p;
    if(hw_pid==1)
        return i;
    p = find_get_pid(hw_pid);
    if(p==NULL)
        return 0;

    ts=pid_task(p,PIDTYPE_PID);
    struct task_struct *parent;
    parent=ts->parent;
    i++;
    P_long(parent->pid,i);
}
static void get_parent(int hw_pid,int *all_pid,int i)
{

    struct task_struct *ts;
    struct pid *p;
    p = find_get_pid(hw_pid);
    if(p==NULL)
        return;
    ts=pid_task(p,PIDTYPE_PID);
    all_pid[i]=hw_pid;
    if(hw_pid==1)
        return;
    struct task_struct *parent;
    parent=ts->parent;
    i=i+1;
    get_parent(parent->pid,all_pid,i);
}
static void P_function(char *trns_msg,int *all_pid,int length,int space)
{
    char int_to_str [10];
    struct task_struct *ts;
    struct pid *p;
    int i;
    int loop;
    for(i=length; i>=0; i--) {
        p = find_get_pid(all_pid[i]);
        ts=pid_task(p,PIDTYPE_PID);
        for(loop=0; loop<space; loop++)
            strcat(trns_msg," ");
        strcat(trns_msg,ts->comm);
        strcat(trns_msg,"(");
        sprintf(int_to_str,"%d",ts->pid);
        strcat(trns_msg,int_to_str);
        strcat(trns_msg,")");
        strcat(trns_msg,"\n");
        space=space+4;
        printk("every is %d\n",ts->pid);
    }
}


static void hello_nl_recv_msg(struct sk_buff *skb)
{



    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg="from kernel";
    int res;
    printk(KERN_INFO "Entering: %s\n", __FUNCTION__);







//recieve from client and decompose it
    char hw_option;
    int hw_pid=0;
    int recieve_long;
    int factor=1;
    nlh=(struct nlmsghdr*)skb->data;
    char *msg_from_user = (char*)nlmsg_data(nlh);
    recieve_long=strlen(msg_from_user);

    hw_option=msg_from_user[1];
    if(msg_from_user[2]=='0')
        hw_pid=task_pid_nr(current);
    else {
        for(; recieve_long>2; recieve_long--) {
            hw_pid=hw_pid+factor*(msg_from_user[recieve_long-1]-48);
            factor=factor*10;
        }
    }


    printk(KERN_INFO "Netlink received msg payload:%s %c %d\n",msg_from_user,hw_option,hw_pid);



    char trns_msg [10000];
    memset(trns_msg, 0, sizeof(char)*10000);

    int family=0;
    if(hw_option=='c'||hw_option=='C') {
        C_function(hw_pid,trns_msg,0);
    } else if(hw_option=='s'||hw_option=='S') {
        S_function(hw_pid,trns_msg);
    } else if(hw_option=='p'||hw_option=='P') {
        family=P_long(hw_pid,0);
        printk("first is %d\n",family);
        int all_pid [family+1];
        get_parent(hw_pid,all_pid,0);
        printk("all is %d\n",all_pid[3]);
        P_function(trns_msg,all_pid,family,0);
    }






    msg = trns_msg;
    msg_size=strlen(msg);



    pid = nlh->nlmsg_pid; /*pid of sending process */

    skb_out = nlmsg_new(msg_size,0);

    if(!skb_out) {

        printk(KERN_ERR "Failed to allocate new skb\n");
        return;

    }
    nlh=nlmsg_put(skb_out,0,0,NLMSG_DONE,msg_size,0);
    NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
    strncpy(nlmsg_data(nlh),msg,msg_size);

    res=nlmsg_unicast(nl_sk,skb_out,pid);

    if(res<0)
        printk(KERN_INFO "Error while sending bak to user\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////

static int __init hello_init(void)
{

    printk("Entering: %s\n",__FUNCTION__);
//This is for 3.6 kernels and above.
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
//nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, hello_nl_recv_msg,NULL,THIS_MODULE);
    if(!nl_sk) {

        printk(KERN_ALERT "Error creating socket.\n");
        return -10;

    }

    return 0;
}

static void __exit hello_exit(void)
{

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");