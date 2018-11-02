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
#include <math.h>
#define ENTRY_NAME "elevator"
#define PERMS 0644
#define PARENT NULL

MODULE_LICENSE("GPL");

// extern function pointers
extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int passenger_type, int start_floor, int destination_floor);
extern long (*STUB_stop_elevator)(void);

typedef enum{ OFFLINE, IDLE, LOADING, UP, DOWN } States;

/* 	declare start of list, can optionally be w/in struc
<<<<<<< HEAD
 *	floor offset by one
 */
 // refer to Floors[1-10], ignore Floor[0]
static struct list_head Floors[11];
=======
 *	floor offset by one, ex. floor1 == floor[0]
 */ 
struct list_head Floors[11];
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7
static int elevatorSignal[11];

// object has to have list_head embedded in it
typedef struct person {
		struct list_head floor;
		int passenger_type;
		int start_floor;
		int destination_floor;
} Person;

typedef struct elevator {
		struct list_head occupancy;
		double Totalweight;
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
static struct task_struct * elevator_thread;

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
		sprintf(message + strlen(message), "%f", elevator.Totalweight);
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

<<<<<<< HEAD
/* ########################## PROC FUNCTIONS END ############################ */


/* ########################## THREAD FUNCTIONS ############################ */

double getWeight(int type)
{
	switch (type)
	{
		case 0:
			return 1.0;
			break;
		case 1:
			return 0.5;
			break;
		case 2:
			return 2.0;
			break;
		case 3:
			return 2.0;
			break;
	}
	return 0.0;
}
=======
void move_elevator(int dir)
{
	while( elevator.current_floor != elevator.next_floor)
	{
		ssleep(2);
		
		if (dir == 3)
			elevator.current_floor++;
		else
			elevator.current_floor--;
	}

	elevator.elevator_state = LOADING;
}

>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7

int run_elevator(void *data)
{
		printk(KERN_INFO "run_elevator is running\n");
		while (!kthread_should_stop())
		{
				int i;
				printk("in k thread loop\n");

				//iterate throughout elevatorSignal and check
				//if button has been push
				//if button has been pushed, then update elevator.next_floor
				//is updated
<<<<<<< HEAD
				for (i = 1; i < 11; i++)
=======
				for (i = 0; i < 11; i++)
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7
				{
						if (elevatorSignal[i] == 1 && elevator.numofPeople <= 0)
						{
								//Go to this floor
								elevator.next_floor = i;

<<<<<<< HEAD
=======

								if (elevator.current_floor < elevator.next_floor)
									elevator.elevator_state = UP;
								else
									elevator.elevator_state = DOWN;

>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7
								elevatorSignal[i] = 0;
								break;
						}
				}

<<<<<<< HEAD
				printk("no elev signal, go to person in elev\n");

				if (elevator.next_floor > elevator.current_floor) elevator.elevator_state = UP;
				else if (elevator.next_floor < elevator.current_floor) elevator.elevator_state = DOWN;


				printk("elevator.next_floor = %d and i = %d", elevator.next_floor, i);
				if (elevator.elevator_state == UP)
				{
					printk("in UP\n");

						while (elevator.current_floor != elevator.next_floor)
						{
								ssleep(2);
								elevator.current_floor++;
						}
						elevator.elevator_state = LOADING;

				}
				else if (elevator.elevator_state == DOWN)
				{
					printk("in DOWN\n");

						while (elevator.current_floor != elevator.next_floor)
						{
								ssleep(2);
								elevator.current_floor--;
						}
						elevator.elevator_state = LOADING;
				}
        // drop off, then add
        // assumes current_floor [1-11]

        // drop off
        list_for_each_safe(temp, dummy, &elevator.occupancy)
        {
          tempP = list_entry(temp, Person, floor);
          if (tempP->destination_floor == elevator.current_floor)
          {
						elevator.numofPeople--;
						elevator.Totalweight-=getWeight(tempP->passenger_type);
            list_del(temp);
            kfree(tempP);
            printk("dropped off person!!!");
          }
        }

        // add to elevator.occupancy
        list_for_each_safe(temp, dummy, &Floors[elevator.current_floor])
        {
					elevator.numofPeople++;
					elevator.Totalweight+=getWeight(tempP->passenger_type);
          list_move_tail(temp, &elevator.occupancy);
        }
        list_for_each(temp, &elevator.occupancy)
        {
            tempP = list_entry(temp, Person, floor);
            elevator.next_floor = tempP->destination_floor;
        }

=======
				if (elevator.elevator_state != LOADING && elevator.elevator_state != IDLE)
					move_elevator(elevator.elevator_state);


				//Drop of the passengers				
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7
				ssleep(1);
		}

		return 0;
}

/* ########################## THREAD FUNCTIONS END ############################ */


/* ########################## SYSTEM CALL FUNCTIONS ############################ */

long my_start_elevator(void)
{
		printk(KERN_NOTICE "%s: You called start_elevator\n" ,__FUNCTION__);
		// Initializes elevator structure for use

		INIT_LIST_HEAD(&elevator.occupancy);
		elevator.Totalweight = 0;
		elevator.numofPeople = 0;
		elevator.elevator_state = IDLE;
		elevator.current_floor = 1;
<<<<<<< HEAD
		elevator.next_floor = 1;
=======
		elevator.next_floor = 0;
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7

		elevator_thread = kthread_run(run_elevator, NULL, "Elevator Thread");

		if (IS_ERR(elevator_thread))
		{
<<<<<<< HEAD
				printk("ERROR! run_elevator\n");
				return PTR_ERR(elevator_thread);
=======
			printk("ERROR! run_elevator\n");
			return PTR_ERR(elevator_thread);
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7
		}

		return 0;
}

long my_issue_request(int passenger_type, int start_floor, int destination_floor)
{
		printk(KERN_NOTICE "%s: You called issue_request: %d\n", __FUNCTION__,passenger_type);
		// Creates a Dynamic Person
		person = kmalloc(sizeof(Person), __GFP_RECLAIM);
		person->passenger_type = passenger_type;
		person->start_floor = start_floor;
		person->destination_floor = destination_floor;

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
		int ret;
		printk(KERN_NOTICE "%s: You called stop_elevator\n", __FUNCTION__);
<<<<<<< HEAD
		ret = kthread_stop(elevator_thread);
		if (ret != -EINTR)
				printk("run elevator has stopped\n");
=======
	   	ret = kthread_stop(elevator_thread);
		if (ret != -EINTR)
			printk("run elevator has stopped\n");
		else
			printk("run elevator has not stopped\n");
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7

		return 0;
}

/* ########################## SYSTEM CALL FUNCTIONS END ############################ */


/* ########################## SYSTEM CALL FUNCTIONS ############################ */

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

<<<<<<< HEAD
		// init list to empty
		for (i=1; i < 11; i++)
=======
		// init list to empty  	
		for (i=0; i < 11; i++)
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7
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

<<<<<<< HEAD
/* USE FOR LOOP TO DELETE ALL PEOPLE IN FLOORS AND ELEVATOR
		list_for_each(temp, &Floors[0]) {
				tempP = list_entry(temp, Person, floor);
				printk(KERN_INFO "person type: %d\nstart_floor: %d\ndestination_floor: %d\n",
								tempP->passenger_type, tempP->start_floor, tempP->destination_floor);
		}

=======
>>>>>>> 89fef5c39860db58a0ea2a09a332050d9e411ee7
		list_for_each_safe(temp, dummy, &Floors[0]) {
				tempP=list_entry(temp, Person, floor);

				list_del(temp);
				kfree(tempP);
		}
*/
		STUB_start_elevator = NULL;
		STUB_issue_request = NULL;
		STUB_stop_elevator = NULL;
}

module_init(elevator_init);
module_exit(elevator_exit);

/* ########################## SYSTEM CALL FUNCTIONS END ############################ */
