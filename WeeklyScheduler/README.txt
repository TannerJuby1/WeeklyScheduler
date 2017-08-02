*******************************************************
*  Name      :  Tanner Juby        
*  Student ID:  101680435               
*  Class     :  CSCI 4761           
*  LAB#      :  2                
*  Due Date  :  April 3, 2017
*******************************************************


                 Read Me


*******************************************************
*  Description of the program
*******************************************************

Server/Client program using TCP socket connection.

Server acts as a calendar that a client can use to:
	Add a user	Delete a user	Modify user information: Name, password, contact 
		phone number, and email address	User login authentication with user name and 
		password	Add a new appointment	Remove an appointment	Update an existing appointment	Check a user’s appointment time conflict	Display a user’s the appointment for a specific 
		time or a time range


*******************************************************
*  Source files
*******************************************************

Name:  server.cpp
   Server side of the program.

Name:  client.cpp
   Client side of the program.

   
   
*******************************************************
*  Circumstances of programs
*******************************************************

   This program works and runs as intended.

   ERROR: Problem after extracting project. User.csv, 
	Appointment.csv, and Sessions.csv might possibly 
	end up with an extra return character after 
	extraction. 
	TEST ERROR: As the client, do the add_account 
		action as your first action. If you do 
		not receive anything back, shut down server
		and client. Go into the User.csv file and 
		make sure there is only one empty line
		after the last record. 

   ERROR: On csegrid, if a single client makes multiple 
	calls over and over again in the same session,
	the return information from the server appends
	instead of rewriting. For example, on the first
	query, the server will send back the answer for 
	the first query. On the second query the server 
	will send back the answer for the first and second
	query. 
	This doesn’t happen on all consecutive calls, but
	it does happen on a few.
	Did not have time to fix this, but the 
	overall functionality still works and you still
	get the same information you want.

   I included some screen shots of the program working on 
	the csegrid as well as the program working on my
	computer
  
   The program was tested on 
	Computer: My Personal Mac Book Pro
	C++ Version: 4.2.1

   The program was also tested on:
	Computer: CSEGrid
	C++ Version: 4.4.7


*******************************************************
*  How to build and run the program
*******************************************************

1. Uncompress the tar file
        tar -xvzf JUB0435.tar.gz

   Now you should see a directory named homework with the files:
	server.cpp
	server.hpp
	client.cpp
	client.hpp
        makefile
	README.txt
	Appointment.csv
	User.csv
	Sessions.csv
	CSE Grid Screen Shots (folder)
	Personal Computer Screen Shots (folder)

2. Build the program.

    Change to the directory that contains the file by:
    cd WeeklyScheduler

    Compile the program by:
    make

3. Figure out host name
   Type ‘hostname’ into the terminal window to get the computers HOSTNAME

4. Run the server side (from one terminal window):
   ./server

5. Run the client side (from another terminal window:
   ./client <hostname from step 3>

6. Delete the obj files, executables, and core dump by
   ./make clean
