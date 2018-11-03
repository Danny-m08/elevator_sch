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

// extern function pointers
extern long (*STUB_start_elevator)(void);
extern long (*STUB_issue_request)(int passenger_type, int start_floor, int destination_floor);
extern long (*STUB_stop_elevator)(void);

typedef enum{ OFFLINE, IDLE, LOADING, UP, DOWN } States;

/* 	declare start of list, can optionally be w/in struc
 *	floor offset by one, ex. floor1 == floor[0]
 */
struct list_head Floors[11];
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
		int weightUnit;
		int decimalWeightUnit;
		int numofPeople;
		States elevator_state;
		States prev_state;
		int current_floor;
		int next_floor;
} Elevator;

static Person *person;
static struct list_head *temp;
static struct list_head *dummy;
static Person *tempP;
static Elevator elevator;
static struct task_struct * elevator_thread;
//static int isFloatingPoint = 0;
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

		int i;
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
/*
		if (isFloatingPoint == 1)
		{
				sprintf(message + strlen(message), "%d.5", elevator.Totalweight - 1);
		}
		else
				sprintf(message + strlen(message), "%d", elevator.Totalweight);
*/
		sprintf(message + strlen(message), "%d.%d", elevator.weightUnit, elevator.decimalWeightUnit);
		strcat(message, "\n");


		i = 0;
		list_for_each(temp, &elevator.occupancy)
		{
				tempP = list_entry(temp, Person, floor);

				sprintf(message + strlen(message), "Person %d: ", i++);
				sprintf(message + strlen(message), "\tPerson.type = %d", tempP -> passenger_type);
				sprintf(message + strlen(message), "\tPerson.start_floor = %d", tempP -> start_floor);
				sprintf(message + strlen(message), "\tPerson.dest_floor = %d\n", tempP -> destination_floor);
		}
		printk(KERN_INFO  "%s", message);

		return 0;
}


ssize_t elevator_read(struct file * sp_file, char __user * buf, size_t size, loff_t * offset)
{
		int len = strlen(message);
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

void addWeight(int type, int *weightUnit, int *decimalWeightUnit)
{
switch (type)
	{
  	case 0:
    	*weightUnit = *weightUnit + 1;
    	break;
		case 1:
    	*decimalWeightUnit = *decimalWeightUnit + 5;
			break;
  	case 2:
    	*weightUnit = *weightUnit + 2;
    	break;
  	case 3:
    	*weightUnit = *weightUnit + 2;
    	break;
	}
	// if decimalWeightUnit gets to 10, carryover 1
	if (*decimalWeightUnit == 10)
		{
			*weightUnit = *weightUnit + 1;
			*decimalWeightUnit=0;
		}
}

void loseWeight(int type, int *weightUnit, int *decimalWeightUnit)
{
switch (type)
	{
  	case 0:
    	*weightUnit = *weightUnit - 1;
    	break;
		case 1:
    	*decimalWeightUnit = *decimalWeightUnit - 5;
			break;
  	case 2:
    	*weightUnit = *weightUnit - 2;
    	break;
  	case 3:
    	*weightUnit = *weightUnit - 2;
    	break;
	}
	// if decimalWeightUnit gets to -5, leave 5 and decr weightUnit-1
	if (*decimalWeightUnit == -5)
		{
			*weightUnit = *weightUnit - 1;
			*decimalWeightUnit=5;
		}
}

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

		elevator.prev_state = elevator.elevator_state;
		elevator.elevator_state = LOADING;
}
/*int loadUp(int i)
  {
  Person * tempPerson;

  int j;
  j = 0;
  list_for_each_safe(temp, dummy, &Floors[i])
  {
  tempP = list_entry(temp, Person, floor);
  j++;
  }

  return 0;


  }*/

/*
	function takes the index of a specific floor and the person in the list
	that you want to bring on board, seems to work
	to specific the first person of the queue pass in 0

*/
int  loadUp(int i, int itr)
{
		Person * tempPerson;

		int j;
		j = 0;
		//iterate throughout a specific floor
		list_for_each_safe(temp, dummy, &Floors[i])
		{
				tempP = list_entry(temp, Person, floor);

				// once we reached the itr-th person
				if (j == itr)
				{
						//create a new Person object
						//copy the data from the specific person you want from the floor list
						// to the new Person
						// add new person object you just created to the elevator list
						tempPerson = kmalloc(sizeof(Person), __GFP_RECLAIM);
						tempPerson -> passenger_type = tempP -> passenger_type;
						tempPerson -> start_floor = tempP -> start_floor;
						tempPerson -> destination_floor	= tempP -> destination_floor;
						list_add_tail(&tempPerson -> floor, &elevator.occupancy);

						elevator.numofPeople++;

/*
						if (tempPerson -> passenger_type == 0)	//Adult
								elevator.Totalweight += 1;
						else if (tempPerson -> passenger_type == 1) // child
						{
								if (isFloatingPoint == 0)
										isFloatingPoint	= 1;
								else
										isFloatingPoint	= 0;

								elevator.Totalweight += 1;
						}
						else if ( tempPerson -> passenger_type == 2 || tempPerson -> passenger_type == 3)
								elevator.Totalweight += 2;
*/
 						addWeight(tempPerson->passenger_type, &(elevator.weightUnit), &(elevator.decimalWeightUnit));



						//delete the old person from the floor list
						list_del(temp);
						kfree(tempP);
						return 1;
				}

				j++;
		}

		return 0;
}

void remove_passengers(void)
{
		//iterate throughout the elevator list
		list_for_each_safe(temp,dummy, &elevator.occupancy)
		{
				tempP = list_entry(temp, Person, floor);

				// if one of the people in elevator list destination floor is this floor
				// boot the person out
				if (tempP -> destination_floor == elevator.current_floor)
				{
/*
						if (tempP -> passenger_type == 1)	//if passenger is child
						{
								if (isFloatingPoint == 1)	// turn and/off if it is a floating point to account for
										isFloatingPoint = 0; // 0.5 weight
								else
										isFloatingPoint = 1;
						}

						// if passenger is room service or bellhoop, subtract their weight from total weight
						if (tempP -> passenger_type	== 2 || tempP -> passenger_type	== 3)
								elevator.Totalweight -= 2;
						else
								elevator.Totalweight--;	//other wise substract regularly for child and adult
*/
						loseWeight(tempP->passenger_type, &(elevator.weightUnit), &(elevator.decimalWeightUnit));
						list_del(temp);	//delete person and decrement number of people in the elevator
						kfree(tempP);
						elevator.numofPeople--;
				}
		}

}

int run_elevator(void *data)
{
		printk(KERN_INFO "run_elevator is running\n");
		while (!kthread_should_stop())
		{
				int i;
				printk("in k thread loop\n");

				/*
				 	use elevatorSignal if the elevator list is empty,
					go to the first floor that button has been pushed,
					if poeple are already in the elevator, they take priority and
					elevate has to service them first (hence the else statement)
				 */
				if (elevator.numofPeople == 0)
				{
						for (i = 0; i < 11; i++)
						{
								if (elevatorSignal[i] == 1)	//someone has pushed the button
								{
										//Go to this floor next
										elevator.next_floor = i;

										// if the next floor is above the current floor go UP, otherwise down
										if (elevator.current_floor < elevator.next_floor)
												elevator.elevator_state = UP;
										else
												elevator.elevator_state = DOWN;

										// we know that the button has been push. turn of the button
										elevatorSignal[i] = 0;
										break;
								}
						}
				}
				else
				{
						/*
							if the elevator is loading, iterate throughout the elevator list
							and check for the first person that wants to get off in the same
							direction that elevator was heading in the first place ( hend elevator.prev_state)
						*/
						if (elevator.elevator_state == LOADING)
						{
								list_for_each(temp, &elevator.occupancy)
								{
										tempP = list_entry(temp, Person, floor);

										if (elevator.prev_state == UP && tempP -> destination_floor > elevator.current_floor)
										{
												// elevator was going UP and the next person in the elevator list
												// wants to go UP than the current floor, GO UP
												elevator.next_floor = tempP -> destination_floor;
												elevator.elevator_state = UP;
												break;
										}
										else if (elevator.prev_state == DOWN && tempP -> destination_floor < elevator.current_floor)
										{
												// elevator was going DOWN and the next person in the elevator list
												// wants to go DOWN than the current floor, GO DOWN
												elevator.next_floor = tempP -> destination_floor;
												elevator.elevator_state = DOWN;
												break;
										}
										else if (elevator.prev_state == UP && tempP -> destination_floor < elevator.current_floor)
										{
												// elevator was go UP and the next person in the elevator list wants to go DOWN
												// go DOWN
												elevator.next_floor = tempP -> destination_floor;
												elevator.elevator_state = DOWN;
												break;
										}
										else if (elevator.prev_state == DOWN && tempP -> destination_floor > elevator.current_floor)
										{
												// elevator was go DOWN and the next person in the elevator list wants to go UP
												// go UP
												elevator.next_floor = tempP -> destination_floor;
												elevator.elevator_state = UP;
												break;
										}
								}
						}
				}

				// elevator.elevator_state has to be UP or DOWN to move
				if (elevator.elevator_state != LOADING && elevator.elevator_state != IDLE)
						move_elevator(elevator.elevator_state);


				//check for any passengers that want to get off and Drop of the passengers
				remove_passengers();

				// we have stop at a specific floor and dropped of passengers, time to load up bois
				// this function does not take care of weight yet
				if (elevator.elevator_state == LOADING)
						loadUp(elevator.current_floor, 0);


				ssleep(1);
		}

		return 0;
}

long my_start_elevator(void)
{
		printk(KERN_NOTICE "%s: You called start_elevator\n" ,__FUNCTION__);
		// Initializes elevator structure for use

		INIT_LIST_HEAD(&elevator.occupancy);
		elevator.weightUnit = 0;
		elevator.decimalWeightUnit = 0;
		elevator.numofPeople = 0;
		elevator.elevator_state = IDLE;
		elevator.prev_state = UP;
		elevator.current_floor = 1;
		elevator.next_floor = 0;

		elevator_thread = kthread_run(run_elevator, NULL, "Elevator Thread");

		if (IS_ERR(elevator_thread))
		{
				printk("ERROR! run_elevator\n");
				return PTR_ERR(elevator_thread);
		}

		return 0;
}



long my_issue_request(int passenger_type, int start_floor, int destination_floor)
{
		int i;
		i = 0;
		printk(KERN_NOTICE "%s: You called issue_request: %d\n", __FUNCTION__,passenger_type);
		// Creates a Dynamic Person
		person = kmalloc(sizeof(Person), __GFP_RECLAIM);
		person->passenger_type = passenger_type;
		person->start_floor = start_floor;
		person->destination_floor = destination_floor;

		//add person to specified floor
		list_add_tail(&person->floor, &Floors[ person->start_floor ]);
		elevatorSignal[person->start_floor] = 1;

		list_for_each(temp, &Floors[person->start_floor])
		{
				tempP = list_entry(temp, Person, floor);

				printk("Person %d: ", i++);
				printk("\tPerson.type = %d", tempP -> passenger_type);
				printk("\tPerson.start_floor = %d", tempP -> start_floor);
				printk("\tPerson.dest_floor = %d\n", tempP -> destination_floor);
		}

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

		// this while loop is to make sure the thread does not stop until
		// everyone has been serviced in the elevator
		//while (elevator.numOfPeople != 0) { ; }

		printk(KERN_NOTICE "KThread stopging elevator.numOfPeople: %d\n", elevator.numofPeople);
		ret = kthread_stop(elevator_thread);
		if (ret != -EINTR)
				printk("run elevator has stopped\n");
		else
				printk("run elevator has not stopped\n");

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
		for (i=0; i < 11; i++)
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
