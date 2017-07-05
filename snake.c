/*snake*/
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#define initSPEED 15000

void interrupt (*int8save) (void);
void interrupt myInt8(void);
void interrupt (*int9save) (void);
void interrupt myInt9(void);

volatile int time;
int counter, counter2;
char ScanCode=0;
char dir; 

typedef struct node{
	int X;	// col
	int Y;	// row
	struct node* next;
	struct node* prev;
}node;

typedef struct Snake{
	node* head;
	node* tail;
	int score;
	node* apple;
}Snake,*PSnake;

void HomeScreen();
void pressed();
void exitGame(char* msg);
void playGame();
void openWindow();
void createApple(PSnake mySnake);
int checkClash(node*,int,int);
void addNode(PSnake);
PSnake createSnake();
void printPixel(int row, int col, char color);
void delSnake(PSnake mySnake);
void setSpeed(int speed);
void Sound( int hertz );
void ChangeSpeaker( int status );

void main(){
	int8save = getvect(8);		
	setvect(8,myInt8);				
	int9save = getvect(9);			
	setvect(9,myInt9);	
	setSpeed(initSPEED);

	HomeScreen();
}

void setSpeed(int speed){
	asm{
		MOV AL,36H 		
        OUT 43H,AL
        MOV BX,speed   	
        MOV AL,BL      
        OUT 40H,AL
        MOV AL,BH       
        OUT 40H,AL
	}
}

void playGame(){
	PSnake mySnake;
	int X,Y, speed=initSPEED;
	//int flag=0;
	node* tmp;
	srand(time);
	openWindow();
	mySnake = createSnake();
		
	counter=0;
	while (ScanCode != 1){
		if (counter == 2){
			X = mySnake->head->X;
			Y = mySnake->head->Y;
			addNode(mySnake);
			switch(dir){				
				case 72:	// up
				mySnake->head->X = X;
				mySnake->head->Y = Y-1;
				Y = mySnake->head->Y;
				break;
				
				case 75:	// left
				mySnake->head->X = X-1;
				X = mySnake->head->X;
				mySnake->head->Y = Y;
				break;
				
				case 77:	// right
				mySnake->head->X = X+1;
				X = mySnake->head->X;
				mySnake->head->Y = Y;
				break;
				
				case 80:	// down
				mySnake->head->X = X;
				mySnake->head->Y = Y+1;
				Y = mySnake->head->Y;
				break;
			}
			if ((X==320)||(X==0)||(Y==0)||(Y==200)||(checkClash(mySnake->head->next,X,Y) == 1)){
					break;
				}
				else if (checkClash(mySnake->head,mySnake->apple->X,mySnake->apple->Y) == 1){
					printPixel(Y,X,2);
					createApple(mySnake);
					mySnake->score+=10;
					if (speed > 3000){
						speed-=500;
						setSpeed(speed);
					}
					Sound(2800);
					counter2=0;
					while (counter2<=1);
					ChangeSpeaker(0);
				}

				else{
					printPixel(Y,X,2);
					printPixel(mySnake->tail->Y,mySnake->tail->X,0);
					tmp = mySnake->tail;
					mySnake->tail = mySnake->tail->prev;
					free(tmp);
					mySnake->tail->next=NULL;
				}
			counter = 0;
		}
	}
	
	X=mySnake->score;
	delSnake(mySnake);
	openWindow();
	printf("\n\nGAME OVER!!\nScore: %d\n", X);
	ScanCode = 0;
	while ((ScanCode != 28) && (ScanCode != 1));
	ScanCode = 0;
	HomeScreen();

}

PSnake createSnake(){		// one time only
	PSnake mySnake;
	node* tmp;
	int i;
	mySnake=(PSnake)malloc(sizeof(Snake));
	if (!mySnake)
		exitGame("\n\nMemory Error!");
	
	mySnake->head=NULL;
	mySnake->tail=NULL;
	mySnake->score=0;
	dir=77;	// right
	tmp = (node*)malloc(sizeof(node));
	if (!(tmp)){
		delSnake(mySnake);
		exitGame("\n\nMemory Error!");
	}
	tmp->X = 10;
	tmp->Y = 10;
	tmp->next=NULL;
	tmp->prev=NULL;
	mySnake->head = tmp;
	mySnake->tail = tmp;
	for (i=0;i<30;i++){
		tmp=(node*)malloc(sizeof(node));
		if (!tmp){
			delSnake(mySnake);
			exitGame("\n\nMemory Error!");
		}
		tmp->Y = 10;
		tmp->X = 10+i;
		printPixel(10,10+i,2);
		tmp->next = mySnake->head;
		mySnake->head->prev = tmp;
		mySnake->head = tmp;
	}
	createApple(mySnake);
	return mySnake;
}

void HomeScreen(){
	openWindow();
	printf("\nWelcome to Snake game!!\n");
	printf("\n                          ____");
	printf("\n ________________________/ O  \\___/");
	printf("\n<_/_\\_/_\\_/_\\_/_\\_/_\\_/_______/   \\");
	printf("\n\n\nControlls:\n");
	printf("\n* Use arrows to move left, right,\n  up an down.");
	printf("\n Press Esc in game to go back to menu.");		// SC 1
	printf("\n Press Esc in menu to quit.");
	printf("\n\n ============= RULES  =============");
	printf("\n Avoid borders and eating yourself!!");
	printf("\n Eat apples to grow and gain score.");
	printf("\n\nPress Enter to start.");		// SC 28
	ScanCode = 0;
	while ((ScanCode != 28) && (ScanCode != 1));
	if (ScanCode == 28)
		playGame();
	if (ScanCode == 1)
		exitGame("\n\nYou chose to exit.\n\n");

}

void interrupt myInt8(void){
	asm{
		PUSHF
		CALL DWORD PTR int8save		
	}
	time++;
	counter++;
	counter2++;
}

void interrupt myInt9(void){
	asm{
	IN AL, 60h		
	MOV ScanCode, AL
	TEST AL, 80h	
	JNZ key_released
	}
	
	/*Block movment against the direction*/
	if (ScanCode == 72 || ScanCode == 75 || ScanCode == 77 || ScanCode == 80){
		if (((ScanCode == 72) && (dir == 80)) || ((dir == 72) && (ScanCode == 80))){}
		else if (((ScanCode == 75) && (dir == 77)) || ((dir == 75) && (ScanCode == 77))){}
	else
		dir=ScanCode;
	}
	
key_released:
	asm{
	IN AL, 61h
	OR AL, 80h
	OUT 61h, AL
	AND AL, 7Fh
	OUT 61h, AL
	MOV AL, 20h
	OUT 20h, AL
	}
}

void addNode(PSnake mySnake){
	node* tmp = (node*)malloc(sizeof(node));
	if (!tmp){
		delSnake(mySnake);
		exitGame("\n\nMemory Error!");
	}
	tmp->next=mySnake->head;
	mySnake->head->prev=tmp;
	mySnake->head=tmp;
}

void delSnake(PSnake mySnake){
	node* tmp=mySnake->head;
		while(tmp){
		mySnake->head = mySnake->head->next;
		free(tmp);
		tmp = mySnake->head;
	}
	free(mySnake->apple);
	free(mySnake);
}

void exitGame(char* msg){
	setSpeed(0);
	setvect(9,int9save);
	setvect(8,int8save);

asm{
	MOV AX,2
	INT 10h
}
	printf(msg);
}

void openWindow(){
		asm{
		XOR AH,AH
		MOV AL,13h
		INT 10h
	}
}

void createApple(PSnake mySnake){
	int X, Y;
	 
	 /*find empty spot for apple*/
	do{
		X=rand()%318 + 1;
		Y=rand()%198 + 1;
	}while (checkClash(mySnake->head,X,Y) == 1);
		printPixel(Y,X,4);
		mySnake->apple->X = X;
		mySnake->apple->Y = Y;
}

int checkClash(node* head,int X,int Y){
	node* tmp=head;
	while (tmp){
		if (tmp->X == X && tmp->Y == Y)
			return 1;
		tmp=tmp->next;
	}
	return 0;
}

void printPixel(int row, int col, char color){
	asm{
			MOV CX, col
			MOV DX, row
			MOV AL, color
			MOV AH, 0ch
			INT 10h
		}
}

void Sound( int hertz )
	{
	 unsigned divisor = 1193180L / hertz;

	  ChangeSpeaker( 1 );

   //        outportb( 0x43, 0xB6 );
        asm {
          PUSH AX
          MOV AL,0B6h
          OUT 43h,AL
          POP AX
        } // asm


     //       outportb( 0x42, divisor & 0xFF ) ;
        asm {
          PUSH AX
          MOV AX,divisor
          AND AX,0FFh
          OUT 42h,AL
          POP AX
        } // asm


     //        outportb( 0x42, divisor >> 8 ) ;

        asm {
          PUSH AX
          MOV AX,divisor
          MOV AL,AH
          OUT 42h,AL
          POP AX
        } // asm

	     } /*--Sound( )-----*/
		 
void ChangeSpeaker( int status ){
  int portval;
//   portval = inportb( 0x61 );

      portval = 0;
   asm {
        PUSH AX
        MOV AL,61h
        MOV byte ptr portval,AL
        POP AX
       }

    if ( status==1 )
     portval |= 0x03;
      else
       portval &=~ 0x03;
        // outportb( 0x61, portval );
        asm {
          PUSH AX
          MOV AX,portval
          OUT 61h,AL
          POP AX
        } // asm

	} 
	
	