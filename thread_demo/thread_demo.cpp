// thread_demo.cpp : 定义控制台应用程序的入口点。
//  Bounce - Creates a new thread each time the letter 'a' is typed.
//  Each thread bounces a happy face of a different color around
//  the screen. All threads are terminated when the letter 'Q' is
//  entered.
//
//

#include "stdafx.h"

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <conio.h>
#include <process.h>

#define MAX_THREADS 32

#define getrandom(min,max) (short)((rand()%(int)(((max)+1)-(min)))+(min))
int _tmain(int argc, _TCHAR* argv[]);
void KbdFunc(void);
void BounceProc(void *MyID);
void ClearScreen(void);
void WriteTitle(int ThreadNum);

HANDLE hConsoleOut;
HANDLE hRunMutex;
HANDLE hScreenMutex;
int ThreadNr;
CONSOLE_SCREEN_BUFFER_INFO csbiInfo;

int _tmain(int argc, _TCHAR* argv[])
{
	hConsoleOut=GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hConsoleOut,&csbiInfo);
	ClearScreen();
	WriteTitle(0);

	hScreenMutex=CreateMutex(NULL,FALSE,NULL);
	hRunMutex=CreateMutex(NULL,TRUE,NULL);
	ThreadNr =0;

	KbdFunc();

	CloseHandle(hScreenMutex);
	CloseHandle(hRunMutex);
	CloseHandle(hConsoleOut);

	return 0;
}

void ShutDown(void)
{
	while(ThreadNr >0)
	{
		ReleaseMutex(hRunMutex);
		ThreadNr--;
	}

	WaitForSingleObject(hScreenMutex,INFINITE);
	ClearScreen();
}

void KbdFunc(void)
{
	int KeyInfo;
	do
	{
		KeyInfo =_getch();
		if(tolower(KeyInfo)=='a'&&
			ThreadNr<MAX_THREADS)
		{
			ThreadNr++;
			_beginthread(BounceProc,0,&ThreadNr);
			WriteTitle(ThreadNr);
		}
	}while(tolower(KeyInfo)!='q');
	ShutDown();
}

void BounceProc(void *pMyID)
{
	char MyCell,OldCell;
	WORD MyAttrib,OldAttrib;
	char BlankCell=0x20;
	COORD Coords,Delta;
	COORD Old={0,0};
	DWORD Dummy;
	char *MyID =(char*)pMyID;

	srand((unsigned int )*MyID*3);
	Coords.X=getrandom(0,csbiInfo.dwSize.X-1);
	Coords.Y=getrandom(0,csbiInfo.dwSize.Y-1);
	Delta.X=getrandom(-3,3);
	Delta.Y=getrandom(-3,3);
// Set up "happy face" & generate color 
    // attribute from thread number.
    if( *MyID > 16)
        MyCell = 0x01;          // outline face 
    else
        MyCell = 0x02;          // solid face 
    MyAttrib =  *MyID & 0x0F;   // force black background 

    do
    {
        // Wait for display to be available, then lock it.
        WaitForSingleObject( hScreenMutex, INFINITE );

        // If we still occupy the old screen position, blank it out. 
        ReadConsoleOutputCharacter( hConsoleOut, &OldCell, 1, 
                                    Old, &Dummy );
        ReadConsoleOutputAttribute( hConsoleOut, &OldAttrib, 1, 
                                    Old, &Dummy );
        if (( OldCell == MyCell ) && (OldAttrib == MyAttrib))
            WriteConsoleOutputCharacter( hConsoleOut, &BlankCell, 1,
                                         Old, &Dummy );

        // Draw new face, then clear screen lock 
        WriteConsoleOutputCharacter( hConsoleOut, &MyCell, 1, 
                                     Coords, &Dummy );
        WriteConsoleOutputAttribute( hConsoleOut, &MyAttrib, 1, 
                                     Coords, &Dummy );
        ReleaseMutex( hScreenMutex );

        // Increment the coordinates for next placement of the block. 
        Old.X = Coords.X;
        Old.Y = Coords.Y;
        Coords.X += Delta.X;
        Coords.Y += Delta.Y;

        // If we are about to go off the screen, reverse direction 
        if( Coords.X < 0 || Coords.X >= csbiInfo.dwSize.X )
        {
            Delta.X = -Delta.X;
            Beep( 400, 50 );
        }
        if( Coords.Y < 0 || Coords.Y > csbiInfo.dwSize.Y )
        {
            Delta.Y = -Delta.Y;
            Beep( 600, 50 );
        }
    }
    // Repeat while RunMutex is still taken. 
    while ( WaitForSingleObject( hRunMutex, 75L ) == WAIT_TIMEOUT );
}

void WriteTitle( int ThreadNum )
{
    enum { 
        sizeOfNThreadMsg = 80 
    };
    char    NThreadMsg[sizeOfNThreadMsg];

    sprintf_s( NThreadMsg, sizeOfNThreadMsg, 
               "Threads running: %02d.  Press 'A' "
               "to start a thread,'Q' to quit.", ThreadNum );
    SetConsoleTitle( NThreadMsg );
}

void ClearScreen( void )
{
    DWORD    dummy;
    COORD    Home = { 0, 0 };
    FillConsoleOutputCharacter( hConsoleOut, ' ', 
                                csbiInfo.dwSize.X * csbiInfo.dwSize.Y, 
                                Home, &dummy );
}

