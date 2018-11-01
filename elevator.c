#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL

MODULE_LICENSE("GPL");
static struct task_struct *kthread;

// extern function pointers
extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int passenger_type, int start_floor, int destination_floor);
extern long (*STUB_stop_elevator)(void);

typedef enum{ OFFLINE, IDLE, LOADING, UP, DOWN } States;

// declare start of list, can optionally be w/in struct
// floor offset by one, ex. floor1 == floor[0]
struct list_head Floors[10];
static int elevatorSignal[10];

// object has to have list_head embedded in it
typedef struct person {
		struct list_head floor;
		int passenger_type;
		int start_floor;
		int destination_floor;
} Person;

typedef struct elevator {
		struct list_head occupancy;
		int Totalweight;
		int numofPeople;
		States elevator_state;
		int current_floor;
		int next_floor;
} Elevator;

static Person *person;
static struct list_head *temp;
static struct list_head *dummy;
static Person *tempP;
static Elevator elevator;


/* ===========================THREAD_FUNC_BEGIN===========================
   int thread_run(void *data)
   {

   }

   void thread_init_parameter(struct thread_parameter *parm)
   {

   }
   ===========================THREAD_FUNC_END===========================*/


/* ########################## PROC FUNCTIONS ############################ */

static struct file_operations fops;
static char *message;
static int read_p;

int elevator_open(struct inode *sp_inode, struct file *sp_file) {

		printk(KERN_INFO "elevator_open proc is called\n");
		read_p = 1;
		message = kmalloc(sizeof(char) * 2048, __GFP_RECLAIM | __GFP_IO | __GFP_FS);

		if (message == NULL)
		{
				printk(KERN_WARNING "Elevator proc open: message is null\n");
				return -ENOMEM;
		}		

		strcpy(message, "STATE: ");

		switch(elevator.elevator_state)
		{
				case OFFLINE:
						strcat(message, "OFFLINE");
						break;
				case IDLE:
						strcat(message, "IDLE");
						break;
				case LOADING:
						strcat(message, "LOADING");
						break;
				case UP:
						strcat(message, "UP");
						break;
				case DOWN:
						strcat(message, "DOWN");
						break;	
		}

		strcat(message, "\n");
		strcat(message, "CURRENT FLOOR: ");
		sprintf(message + strlen(message), "%d", elevator.current_floor);
		strcat(message, "\n");
		strcat(message, "NEXT FLOOR: ");
		sprintf(message + strlen(message), "%d", elevator.next_floor);
		strcat(message, "\n");
		strcat(message, "OCCUPANCY: ");
		sprintf(message + strlen(message), "%d", elevator.numofPeople);
		strcat(message, "\n");
		strcat (message, "TOTAL WEIGHT: ");
		sprintf(message + strlen(message), "%d", elevator.Totalweight);
		strcat(message, "\n");
		printk(KERN_INFO  "%s", message);

		return 0;
}


ssize_t elevator_read(struct file * sp_file, char __user * buf, size_t size, loff_t * offset)
{
		int len = strlen(message);
		printk(KERN_INFO "len of message is %d\n", len);
		read_p = !read_p;

		if (read_p)
				return 0;

		printk(KERN_INFO "elevator proc called read\n");
		copy_to_user(buf, message, len); 

		return len; 
}

int elevator_release(struct inode * sp_inode, struct file *sp_file)
{
		printk(KERN_INFO "elevator proc called release\n");
		kfree(message);
		return 0; 
}


int run_elevator(void *data)
{

  while (!kthread_should_stop())
    {
	int i;

	//iterate throughout elevatorSignal and check
	//if button has been push
	//if button has been pushed, then update elevator.next_floor
	//is updated
	for (i = 0; i < 10; i++)
	{
		if (elevatorSignal[i] == 1)
		{
			//Go to this floor
			elevator.next_floor = i;
		}
	} 
	

	while (elevator.current_floor != elevator.next_floor)
	{
		ssleep(2);
		elevator.current_floor++;
	}

	elevator.elevator_state = LOADING;
//	elevatorSignal[i] = 0;
    }

	

	return 0;
}

long my_start_elevator(void)
{
		printk(KERN_NOTICE "%s: You called start_elevator\n" ,__FUNCTION__);
		// Initializes elevator structure for use		

		INIT_LIST_HEAD(&elevator.occupancy);
		elevator.Totalweight = 0;
		elevator.numofPeople = 0;
		elevator.elevator_state = IDLE;	
		elevator.current_floor = 1;
		elevator.next_floor = 1;

		return 0;
}



long my_issue_request(int passenger_type, int start_floor, int destination_floor)
{
		printk(KERN_NOTICE "%s: You called issue_request: %d\n", __FUNCTION__,passenger_type);
		// Creates a Dynamic Person
		person = kmalloc(sizeof(Person), __GFP_RECLAIM);
		person->passenger_type = passenger_type;
		person->start_floor = 0;
		person->destination_floor = destination_floor - 1;

		//add person to specified floor
		list_add_tail(&person->floor, &Floors[ person->start_floor ]);
		elevatorSignal[person->start_floor] = 1;

		/*  // build people in appropriate line
			list_add_tail(&person->floor, &Floors[0]);
			printk(KERN_NOTICE "%s: You called issue_request: %d\n", __FUNCTION__,passenger_type);
			*/
		return passenger_type;
}

long my_stop_elevator(void)
{
		printk(KERN_NOTICE "%s: You called stop_elevator\n", __FUNCTION__);
		return 0;
}

static int elevator_init(void)
{
		int i;
		printk(KERN_NOTICE "/proc/%s create\n", ENTRY_NAME);
		fops.open = elevator_open;
		fops.read = elevator_read;
		fops.release = elevator_release;

		if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops))
		{
				printk(KERN_WARNING "ERROR! proc_create\n");
				remove_proc_entry(ENTRY_NAME, NULL);
				return -ENOMEM;
		}
		
		// init list to empty  	
		for (i=0; i < 10; i++)
		{
				INIT_LIST_HEAD(&Floors[i]);
				elevatorSignal[i] = 0;
		}

		STUB_start_elevator = my_start_elevator;
		STUB_issue_request = my_issue_request;
		STUB_stop_elevator = my_stop_elevator;

		return 0;
}

static void elevator_exit(void)
{
		remove_proc_entry(ENTRY_NAME, NULL);
		printk(KERN_NOTICE "Removing /proc/%s.\n", ENTRY_NAME);

		list_for_each(temp, &Floors[0]) {
				tempP = list_entry(temp, Person, floor);
				printk(KERN_INFO "person type: %d\nstart_floor: %d\ndestination_floor: %d\n",
								tempP->passenger_type, tempP->start_floor, tempP->destination_floor);
		}

		list_for_each_safe(temp, dummy, &Floors[0]) {
				tempP=list_entry(temp, Person, floor);

				list_del(temp);
				kfree(tempP);
		}

		STUB_start_elevator = NULL;
		STUB_issue_request = NULL;
		STUB_stop_elevator = NULL;
}

module_init(elevator_init);
module_exit(elevator_exit);
