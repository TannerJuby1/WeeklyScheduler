//
//  server.cpp
//  WeeklyScheduler
//
//  Created by Tanner Juby on 3/15/17.
//  Copyright © 2017 Juby. All rights reserved.
//

#include "server.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include <map>

#define MYPORT 3490    // The port users will be connecting to
#define BACKLOG 100     // How many pending connections queue with hold

/**
 IS LOGGED IN
 
 Checks to see if the user is logged in
 */
bool isLoggedIn(char* id) {
    
    FILE* stream = fopen("Sessions.csv", "r");
    char line[1024];
    while (fgets(line, 1024, stream)) {
        
        char* db_entry = strdup(line);
        char* db_id = strtok(db_entry, "\n");
        
        if (strcmp(db_id, id) == 0) {
            return true;
        }
    }
    
    return false;
}

/**
 LOGIN
 
 Logs the user in and returns their account id
 */
char* login(char* email, char* password) {
    
    char result[1024];
    
    FILE* usersFile = fopen("User.csv", "r");
    char line[1024];
    while (fgets(line, sizeof(line), usersFile)) {
        char* db_entry = strdup(line);
        
        // Get id from
        char* db_id = strtok(db_entry, ",");
        
        // Get email and password
        char* db_email = strtok(NULL, ",");
        
        if (strcmp(db_email, email) == 0) {
            // Email Found
            
            char* db_password = strtok(NULL, ",");
            
            if (strcmp(db_password, password) == 0) {
                // Password for email matches
                
                char* db_first = strtok(NULL, ",");
                char* db_last = strtok(NULL, ",");
                char* db_phone = strtok(NULL, "\n");
                
                strcat(result, "SUCCESS: Login successful. Here is your account information:");
                strcat(result, "\n\tKEY: ");
                strcat(result, db_id);
                strcat(result, "\n\tEMAIL: ");
                strcat(result, db_email);
                strcat(result, "\n\tPASSWORD: ");
                strcat(result, db_password);
                strcat(result, "\n\tFIRST NAME: ");
                strcat(result, db_first);
                strcat(result, "\n\tLAST NAME: ");
                strcat(result, db_last);
                strcat(result, "\n\tPHONE: ");
                strcat(result, db_phone);
                
                char* success = result;
                fclose(usersFile);
                
                // Log the user in
                FILE* oldSessionsFile = fopen("Sessions.csv", "r");
                FILE* newSessionsFile = fopen("tempSessions.csv", "w+");
                
                // Check to see if already logged in
                FILE* currentSessionsFile = fopen("Sessions.csv", "r");
                char sessionsLine[1024];
                while (fgets(sessionsLine, 1024, currentSessionsFile)) {
                    
                    char* sessions_entry = strdup(sessionsLine);
                    char* sessions_id = strtok(sessions_entry, "\n");
                    
                    if (strcmp(db_id, sessions_id) == 0) {
                        // User is already logged in
                        char errorResult[1024] = "";
                        strcat(errorResult, "ERROR: You are already logged in. Here is your account key: ");
                        strcat(errorResult, db_id);
                        char* failure = errorResult;
                        return failure;
                    }
                }
                fclose(currentSessionsFile);
                
                char line[1024];
                
                // Copy all current sessions into the new sessions file
                while (fgets(line, sizeof(line), oldSessionsFile)) {
                    fprintf(newSessionsFile, "%s", line);
                }
                
                // Add in the new line
                fprintf(newSessionsFile, "%s\n", db_id);
                
                fclose(oldSessionsFile);
                fclose(newSessionsFile);
                
                // Delete the old sessions file
                if (remove("Sessions.csv") != 0) {
                    strcat(result, "ERROR: Login unsuccessful. Error occurred");
                    char* failure = result;
                    return failure;
                }
                
                // Rename the new sessions file
                if (rename("tempSessions.csv", "Sessions.csv") != 0) {
                    strcat(result, "ERROR: Login unsuccessful. Error occurred");
                    char* failure = result;
                    return failure;
                }
                
                return success;
            }
        }
    }
    
    // No record found
    strcat(result, "ERROR: Login unsuccessful. Incorrect email/password");
    char* failure = result;
    fclose(usersFile);
    
    return failure;
}

/**
 LOGOUT
 
 Logs the user out of the app
 */
bool logout(char* id) {
    
    FILE* oldSessionsFile = fopen("Sessions.csv", "r");
    FILE* newSessionsFile = fopen("tempSessions.csv", "w+");
    
    char line[1024];
    while (fgets(line, sizeof(line), oldSessionsFile)) {
        
        char* db_entry = strdup(line);
        char* db_id = strtok(db_entry, "\n");
        
        // Copy all lines into the new sessions file except the one user being logged out
        if (strcmp(db_id, id) != 0) {
            fprintf(newSessionsFile, "%s", line);
        }
    }
    
    fclose(oldSessionsFile);
    fclose(newSessionsFile);
    
    // Delete the old sessions file
    if (remove("Sessions.csv") != 0) {
        return false;
    }
    
    // Rename the new sessions file
    if (rename("tempSessions.csv", "Sessions.csv") != 0) {
        return false;
    }
    
    return true;
    
}

/**
 CREATE ACCOUNT
 
 Creates a new account
 */
char* createAccount(char* email, char* password, char* firstName, char* lastName, char* phone) {
    
    char result[1024];
    
    FILE* currentUsersFile = fopen("User.csv", "r");
    
    char currentUserline[1024];
    
    // Check to see if the user's email already exists
    while (fgets(currentUserline, sizeof(currentUserline), currentUsersFile)) {
        char* db_entry = strdup(currentUserline);
        
        strtok(db_entry, ","); // Omit the id
        // Get email and password
        char* db_email = strtok(NULL, ",");
        
        if (strcmp(db_email, email) == 0) {
            strcat(result, "ERROR: A user with that email already exists");
            char* failure = result;
            return failure;
        }
    }
    fclose(currentUsersFile);

    // Create the new user
    FILE* oldUsersFile = fopen("User.csv", "r");
    FILE* newUsersFile = fopen("newUser.csv", "w+");
    
    char oldUserLine[1024];
    
    int newUserId = 0;

    // Copy all of the information from old users file to the new users file
    while (fgets(oldUserLine, sizeof(oldUserLine), oldUsersFile)) {
        char* db_entry = strdup(oldUserLine);
        char* db_id = strtok(db_entry, ",");
        
        // Set the new user id
        int db_id_int = atoi(db_id);
        newUserId = db_id_int+1;
        
        // Copy line
        fprintf(newUsersFile, "%s", oldUserLine);
    }
    
    // Add in the new user
    fprintf(newUsersFile, "%d,%s,%s,%s,%s,%s\n", newUserId, email, password, firstName, lastName, phone);
    
    // Close the file
    fclose(oldUsersFile);
    fclose(newUsersFile);
    
    // Delete the old users file
    if (remove("User.csv") != 0) {
        strcat(result, "ERROR: Creation unsuccessful. Problem deleting old file.");
        char* failure = result;
        return failure;
    }
    
    // Rename the new users file
    if (rename("newUser.csv", "User.csv") != 0) {
        strcat(result, "ERROR: Creation unsuccessful. Problem saving");
        char* failure = result;
        return failure;
    }
    
    strcat(result, "Success: Account has been created. Here is your key: ");
    
    char newId[32];
    sprintf(newId, "%d", newUserId);
    strcat(result, newId);
    
    char* success = result;
    return success;
}

/**
 DELETE ACCOUNT
 
 Deletes a user account
 */
char* deleteAccount(char* id, char* password) {
    
    char result[1024];
    
    // Check to see if user has correct password
    FILE* currentUserFile = fopen("User.csv", "r");
    char currentLine[1024];
    while (fgets(currentLine, sizeof(currentLine), currentUserFile)) {
        char* db_entry = strdup(currentLine);
        char* db_id = strtok(db_entry, ",");
        strtok(NULL, ","); // omit the email
        char* db_password = strtok(NULL, ",");
        
        if (strcmp(db_id, id) == 0) {
            if (strcmp(db_password, password) != 0) {
                strcat(result, "ERROR: Deletion unsuccessful. The password does not match for this user id");
                char* failure = result;
                return failure;
            }
            break;
        }
    }
    fclose(currentUserFile);
    
    // Delete the user
    FILE* oldUserFile = fopen("User.csv", "r");
    FILE* newUserFile = fopen("newUser.csv", "w+");
    
    char line[1024];
    while (fgets(line, sizeof(line), oldUserFile)) {
        
        char* db_entry = strdup(line);
        char* db_id = strtok(db_entry, ",");
        
        // Copy all lines into the new sessions file except the one user being deleted
        if (strcmp(db_id, id) != 0) {
            fprintf(newUserFile, "%s", line);
        }
    }
    
    fclose(oldUserFile);
    fclose(newUserFile);
    
    // Delete the old user file
    if (remove("User.csv") != 0) {
        strcat(result, "ERROR: Deletion unsuccessful. Problem deleting old file.");
        char* failure = result;
        return failure;
    }
    
    // Rename the new sessions file
    if (rename("newUser.csv", "User.csv") != 0) {
        strcat(result, "ERROR: Deletion unsuccessful. Problem saving");
        char* failure = result;
        return failure;
    }
    
    logout(id);
    
    strcat(result, "SUCCESS: The user has been deleted");
    char* failure = result;
    return failure;
    
}

/**
 EDIT ACCOUNT
 
 Edits a user account
 */
char* editAccount(char* id, char* currentPassword, char* email, char* newPassword, char* firstName, char* lastName, char* phone) {
    
    char result[1024];
    
    // Check to see if user has correct password
    FILE* currentUserFile = fopen("User.csv", "r");
    char currentLine[1024];
    while (fgets(currentLine, sizeof(currentLine), currentUserFile)) {
        char* db_entry = strdup(currentLine);
        char* db_id = strtok(db_entry, ",");
        strtok(NULL, ","); // omit the email
        char* db_password = strtok(NULL, ",");
        
        if (strcmp(db_id, id) == 0) {
            if (strcmp(db_password, currentPassword) != 0) {
                strcat(result, "ERROR: Edit unsuccessful. The password does not match for this user");
                char* failure = result;
                return failure;
            }
            break;
        }
    }
    fclose(currentUserFile);
    
    // Create the new user
    FILE* oldUsersFile = fopen("User.csv", "r");
    FILE* newUsersFile = fopen("newUser.csv", "w+");
    
    char oldUserLine[1024];

    // Copy all of the information from old users file to the new users file
    while (fgets(oldUserLine, sizeof(oldUserLine), oldUsersFile)) {
        char* db_entry = strdup(oldUserLine);
        char* db_id = strtok(db_entry, ",");
        
        if (strcmp(db_id, id) == 0) {
            // Change the user's information
            fprintf(newUsersFile, "%s,%s,%s,%s,%s,%s\n", id, email, newPassword, firstName, lastName, phone);
        } else {
            // Not the same user, just copy the line
            fprintf(newUsersFile, "%s", oldUserLine);
        }
    }
    
    // Close the file
    fclose(oldUsersFile);
    fclose(newUsersFile);
    
    // Delete the old users file
    if (remove("User.csv") != 0) {
        strcat(result, "ERROR: Edit unsuccessful. Problem deleting old file.");
        char* failure = result;
        return failure;
    }
    
    // Rename the new users file
    if (rename("newUser.csv", "User.csv") != 0) {
        strcat(result, "ERROR: Edit unsuccessful. Problem saving");
        char* failure = result;
        return failure;
    }
    
    strcat(result, "Success: Account has been edited.");
    char* success = result;
    return success;
}

/**
 Is Valid Date
 
 Checks to see if the date is valid
 */
bool isValidDate(char* date) {
    
    char* monthString = strtok(date, "-");
    char* dayString = strtok(NULL, "-");
    char* yearString = strtok(NULL, "");
    
    int month = atoi(monthString);
    int day = atoi(dayString);
    int year = atoi(yearString);

    if (month > 12 || month < 1) {
        return false;
    }
    
    if (day > 31 || day < 1) {
        return false;
    }
    
    if (year < 2017) {
        return false;
    }
    
    return true;
}

/**
 Is Valid Time
 
 Checks to see if the date is valid
 */
bool isValidTime(char* time) {
    
    char* hourString = strtok(time, ":");
    char* minuteString = strtok(NULL, "");
    
    int hour = atoi(hourString);
    int minute = atoi(minuteString);
    
    if (hour < 1 || hour > 23) {
        if (strcmp(hourString, "00") != 0 && strcmp(hourString, "0") != 0) {
            return false;
        }
    }
    
    if (minute < 1 || minute > 59) {
        if (strcmp(minuteString, "00") != 0) {
            return false;
        }
    }
    
    if (strcmp(hourString, "01") == 0 || strcmp(hourString, "02") == 0 || strcmp(hourString, "03") == 0 || strcmp(hourString, "04") == 0 || strcmp(hourString, "05") == 0 || strcmp(hourString, "06") == 0 || strcmp(hourString, "07") == 0 || strcmp(hourString, "08") == 0 || strcmp(hourString, "09") == 0) {
        return false;
    }
        
    return true;
}


/**
 Compare Dates
 
 Compares two dates
 
 Returns 1 if date1 is ahead of date2
 Returns -1 if date2 is ahead of date1
 Returns 0 if the same
*/
int compareDates(char* tempDate1, char* tempDate2) {
    
    char date1[1024];
    strcpy(date1, tempDate1);
    char date2[1024];
    strcpy(date2, tempDate2);
    
    char* month1String = strtok(date1, "-");
    char* day1String = strtok(NULL, "-");
    char* year1String = strtok(NULL, "");
    
    int month1 = atoi(month1String);
    int day1 = atoi(day1String);
    int year1 = atoi(year1String);
    
    char* month2String = strtok(date2, "-");
    char* day2String = strtok(NULL, "-");
    char* year2String = strtok(NULL, "");
    
    int month2 = atoi(month2String);
    int day2 = atoi(day2String);
    int year2 = atoi(year2String);
    
    if (year1 > year2) {
        // Date 1 is greater than Date 2
        return 1;
    } else if (year1 < year2) {
        // Date 2 is greater than Date 1
        return 1;
    } else {
        // Years are the same
        
        if (month1 > month2) {
            // Date 1 is greater than Date 2
            return 1;
        } else if (month1 < month2) {
            // Date 2 is greater than Date 1
            return 1;
        } else {
            // Months are the same
            
            if (day1 > day2) {
                // Date 1 is greater than Date 2
                return 1;
            } else if (day1 < day2) {
                // Date 2 is greater than Date 1
                return 1;
            } else {
                // Days are the same
                return 0;
            }
        }
    }
}

/**
 Compare Times
 
 Compares two times
 
 Returns 1 if time1 is ahead of time2
 Returns -1 if time2 is ahead of time1
 Returns 0 if the same
 */
int compareTimes(char* tempTime1, char* tempTime2) {
    
    // tempTime1 9:00
    // tempTime2 8:30
    
    char time1[1024];
    strcpy(time1, tempTime1);
    char time2[1024];
    strcpy(time2, tempTime2);
    
    char* hour1String = strtok(time1, ":");
    char* minute1String = strtok(NULL, "");
    int hour1 = atoi(hour1String);
    int minute1 = atoi(minute1String);
    
    char* hour2String = strtok(time2, ":");
    char* minute2String = strtok(NULL, "");
    int hour2 = atoi(hour2String);
    int minute2 = atoi(minute2String);
    
    if (hour1 > hour2) {
        // Time 1 is greater than Time 2
        return 1;
    } else if (hour2 > hour1) {
        // Time 2 is greater than Time 1
        return -1;
    } else {
        // Hours are the same
        
        if (minute1 > minute2) {
            // Time 1 is greater than Time 2
            return 1;
        } else if (minute2 > minute1) {
            // Time 2 is greater than Time 1
            return -1;
        } else {
            // Minutes are the same
            
            return 0;
        }
    }
}

/**
 CREATE EVENT
 
 Creates a new event for a user
 */
char* createEvent(char* key, char* name, char* date, char* startTime, char* endTime) {
    
    char result[1024];
    
    char tempDate[1024];
    strcpy(tempDate, date);
    char tempStart[1024];
    strcpy(tempStart, startTime);
    char tempEnd[1024];
    strcpy(tempEnd, endTime);
    
    // Validate Date
    if (!isValidDate(tempDate)) {
        strcat(result, "ERROR: The date entered is not valid. Please make sure the date is in the form: MM-DD-YYYY");
        char* failure = result;
        return failure;
    }
    
    // Validate Start Time
    if (!isValidTime(tempStart)) {
        strcat(result, "ERROR: The start time entered is not valid. Please make sure the time is in the form: HH:MM, but DO NOT start with a '0'. (For example, 09:30 should be entered as 9:30, not 09:30)");
        char* failure = result;
        return failure;
    }
    
    // Validate End Time
    if (!isValidTime(tempEnd)) {
        strcat(result, "ERROR: The end time entered is not valid. Please make sure the time is in the form: HH:MM, but DO NOT start with a '0'. (For example, 09:30 should be entered as 9:30, not 09:30)");
        char* failure = result;
        return failure;
    }
    
    // Make sure start time isnt after end time
    
    if (compareTimes(startTime, endTime) == 1) {
        strcat(result, "ERROR: The start time is after the end time");
        char* failure = result;
        return failure;
    }
    
    // Make sure there are no conflicts with user's previous appointments
    FILE* currentAppointmentsFile = fopen("Appointment.csv", "r");
    char currentAppointmentline[1024];
    
    while (fgets(currentAppointmentline, sizeof(currentAppointmentline), currentAppointmentsFile)) {
        
        char* db_entry = strdup(currentAppointmentline);
        
        strtok(db_entry, ","); // Omit the id
        // Get the user id
        char* db_user_id = strtok(NULL, ",");
        
        if (strcmp(db_user_id, key) == 0) {
            
            strtok(NULL, ","); // Omit the Appointment name
            
            char* db_date = strtok(NULL, ",");
            
            if (strcmp(db_date, date) == 0) {
                // Dates are exactly the same. Compare the times
                
                char* db_start_time = strtok(NULL, ",");
                char* db_end_time = strtok(NULL, ",");
                
                // Start time falls in the middle of an appointment
                if ((compareTimes(startTime, db_start_time) == 1 || compareTimes(startTime, db_start_time) == 0) && compareTimes(startTime, db_end_time) == -1 ) {
                    // Start time runs into another appointment
                    strcat(result, "ERROR: The start time conflicts with another appointment");
                    strcat(result, "\n\n\tOld Appointment: ");
                    strcat(result, db_date);
                    strcat(result, " ");
                    strcat(result, db_start_time);
                    strcat(result, " - ");
                    strcat(result, db_end_time);
                    strcat(result, "\n\tNew Appointment: ");
                    strcat(result, date);
                    strcat(result, " ");
                    strcat(result, startTime);
                    strcat(result, " - ");
                    strcat(result, endTime);
                    char* failure = result;
                    return failure;
                    
                } else if (compareTimes(endTime, db_start_time) == 1 && (compareTimes(endTime, db_end_time) == -1 || compareTimes(endTime, db_end_time) == 0)) {
                    // End time runs into another appointment
                    strcat(result, "ERROR: Your end time conflicts with another appointment");
                    strcat(result, "\n\tOld Appointment: ");
                    strcat(result, db_date);
                    strcat(result, " ");
                    strcat(result, db_start_time);
                    strcat(result, " - ");
                    strcat(result, db_end_time);
                    strcat(result, "\n\tNew Appointment: ");
                    strcat(result, date);
                    strcat(result, " ");
                    strcat(result, startTime);
                    strcat(result, " - ");
                    strcat(result, endTime);
                    char* failure = result;
                    return failure;
                    
                } else if ((compareTimes(startTime, db_start_time) == -1 || compareTimes(startTime, db_start_time) == 0) && (compareTimes(endTime, db_end_time) == 1 || compareTimes(endTime, db_end_time) == 0)) {
                    // Appointment wraps another appointment
                    strcat(result, "ERROR: There is another appointment during this time already");
                    strcat(result, "\n\tOld Appointment: ");
                    strcat(result, db_date);
                    strcat(result, " ");
                    strcat(result, db_start_time);
                    strcat(result, " - ");
                    strcat(result, db_end_time);
                    strcat(result, "\n\tNew Appointment: ");
                    strcat(result, date);
                    strcat(result, " ");
                    strcat(result, startTime);
                    strcat(result, " - ");
                    strcat(result, endTime);
                    char* failure = result;
                    return failure;
                }
            }
        }
    }
    fclose(currentAppointmentsFile);
    
    // Create the new appointment
    FILE* oldAppointmentsFile = fopen("Appointment.csv", "r");
    FILE* newAppointmentsFile = fopen("newAppointment.csv", "w+");
    
    char oldAppointmentLine[1024];
    
    int newAppointmentId = 0;
    
    // Copy all of the information from old users file to the new users file
    while (fgets(oldAppointmentLine, sizeof(oldAppointmentLine), oldAppointmentsFile)) {
        char* db_entry = strdup(oldAppointmentLine);
        char* db_id = strtok(db_entry, ",");
        
        // Set the new user id
        int db_id_int = atoi(db_id);
        newAppointmentId = db_id_int+1;
        
        // Copy line
        fprintf(newAppointmentsFile, "%s", oldAppointmentLine);
    }
    
    printf("Adding new Appointment");
    // Add in the new user
    fprintf(newAppointmentsFile, "%d,%s,%s,%s,%s,%s\n", newAppointmentId, key, name, date, startTime, endTime);
    
    // Close the file
    fclose(oldAppointmentsFile);
    fclose(newAppointmentsFile);
    
    // Delete the old users file
    if (remove("Appointment.csv") != 0) {
        strcat(result, "ERROR: Creation unsuccessful. Problem deleting old file.");
        char* failure = result;
        return failure;
    }
    
    // Rename the new users file
    if (rename("newAppointment.csv", "Appointment.csv") != 0) {
        strcat(result, "ERROR: Creation unsuccessful. Problem saving");
        char* failure = result;
        return failure;
    }
    
    strcat(result, "Success: New appointment has been added to your calendar.");
    char* success = result;
    return success;
}

/**
 DELETE EVENT
 
 Deletes an event
 */
char* deleteEvent(char* id, char* key, char* password) {
    
    char result[] = "";
    
    // Check to see if user has correct password
    FILE* currentUserFile = fopen("User.csv", "r");
    char currentLine[1024];
    while (fgets(currentLine, sizeof(currentLine), currentUserFile)) {
        char* db_entry = strdup(currentLine);
        char* db_id = strtok(db_entry, ",");
        strtok(NULL, ","); // omit the email
        char* db_password = strtok(NULL, ",");
        
        if (strcmp(db_id, key) == 0) {
            if (strcmp(db_password, password) != 0) {
                strcat(result, "ERROR: Deletion unsuccessful. The password does not match for this user id");
                char* failure = result;
                return failure;
            }
            break;
        }
    }
    fclose(currentUserFile);
    
    // Delete the appointment
    FILE* oldAppointmentFile = fopen("Appointment.csv", "r");
    FILE* newAppointmentFile = fopen("newAppointment", "w+");
    
    char line[1024];
    while (fgets(line, sizeof(line), oldAppointmentFile)) {
        
        char* db_entry = strdup(line);
        char* db_id = strtok(db_entry, ",");
        
        // Copy all lines into the new sessions file except the one appointment being deleted
        if (strcmp(db_id, id) != 0) {
            fprintf(newAppointmentFile, "%s", line);
        }
    }
    
    fclose(oldAppointmentFile);
    fclose(newAppointmentFile);
    
    // Delete the old user file
    if (remove("Appointment.csv") != 0) {
        strcat(result, "ERROR: Deletion unsuccessful. Problem deleting old file.");
        char* failure = result;
        return failure;
    }
    
    // Rename the new sessions file
    if (rename("newAppointment", "Appointment.csv") != 0) {
        strcat(result, "ERROR: Deletion unsuccessful. Problem saving");
        char* failure = result;
        return failure;
    }
    
    logout(id);
    
    strcat(result, "SUCCESS: The appointment has been deleted");
    char* failure = result;
    return failure;
    
}

/**
 Edit EVENT
 
 Edits an existing event
 */
char* editEvent(char* id, char* key, char* password, char* name, char* date, char* startTime, char* endTime) {
    
    char result[1024];
    
    // Check to see if user has correct password
    FILE* currentUserFile = fopen("User.csv", "r");
    char currentLine[1024];
    while (fgets(currentLine, sizeof(currentLine), currentUserFile)) {
        char* db_entry = strdup(currentLine);
        char* db_id = strtok(db_entry, ",");
        strtok(NULL, ","); // omit the email
        char* db_password = strtok(NULL, ",");
        
        if (strcmp(db_id, key) == 0) {
            if (strcmp(db_password, password) != 0) {
                strcat(result, "ERROR: Query unsuccessful. The password does not match for this user id");
                char* failure = result;
                return failure;
            }
            break;
        }
    }
    fclose(currentUserFile);
    
    char tempDate[1024];
    strcpy(tempDate, date);
    char tempStart[1024];
    strcpy(tempStart, startTime);
    char tempEnd[1024];
    strcpy(tempEnd, endTime);
    
    // Validate Date
    if (!isValidDate(tempDate)) {
        strcat(result, "ERROR: The date entered is not valid. Please make sure the date is in the form: MM-DD-YYYY");
        char* failure = result;
        return failure;
    }
    
    // Validate Start Time
    if (!isValidTime(tempStart)) {
        strcat(result, "ERROR: The start time entered is not valid. Please make sure the time is in the form: HH:MM, but DO NOT start with a '0'. (For example, 09:30 should be entered as 9:30, not 09:30)");
        char* failure = result;
        return failure;
    }
    
    // Validate End Time
    if (!isValidTime(tempEnd)) {
        strcat(result, "ERROR: The end time entered is not valid. Please make sure the time is in the form: HH:MM, but DO NOT start with a '0'. (For example, 09:30 should be entered as 9:30, not 09:30)");
        char* failure = result;
        return failure;
    }
    
    // Make sure start time isnt after end time
    
    if (compareTimes(startTime, endTime) == 1) {
        strcat(result, "ERROR: The start time is after the end time");
        char* failure = result;
        return failure;
    }
    
    // Make sure there are no conflicts with user's previous appointments
    FILE* currentAppointmentsFile = fopen("Appointment.csv", "r");
    char currentAppointmentline[1024];
    
    while (fgets(currentAppointmentline, sizeof(currentAppointmentline), currentAppointmentsFile)) {
        
        char* db_entry = strdup(currentAppointmentline);
        
        char* db_id = strtok(db_entry, ","); // Omit the id
        // Get the user id
        char* db_user_id = strtok(NULL, ",");
        
        if (strcmp(db_id, id) != 0) {
            // Only compare edited appointment with other appointments, not the old one.
            
            if (strcmp(db_user_id, key) == 0) {
                
                strtok(NULL, ","); // Omit the Appointment name
                
                char* db_date = strtok(NULL, ",");
                
                if (strcmp(db_date, date) == 0) {
                    // Dates are exactly the same. Compare the times
                    
                    char* db_start_time = strtok(NULL, ",");
                    char* db_end_time = strtok(NULL, ",");
                    
                    // 1  : 1 ahead of 2
                    // -1 : 1 behind 2
                    
                    // Start time falls in the middle of an appointment
                    if ((compareTimes(startTime, db_start_time) == 1 || compareTimes(startTime, db_start_time) == 0) && compareTimes(startTime, db_end_time) == -1 ) {
                        // Start time runs into another appointment
                        strcat(result, "ERROR: The start time conflicts with another appointment");
                        strcat(result, "\n\n\tOld Appointment: ");
                        strcat(result, db_date);
                        strcat(result, " ");
                        strcat(result, db_start_time);
                        strcat(result, " - ");
                        strcat(result, db_end_time);
                        strcat(result, "\n\tNew Appointment: ");
                        strcat(result, date);
                        strcat(result, " ");
                        strcat(result, startTime);
                        strcat(result, " - ");
                        strcat(result, endTime);
                        char* failure = result;
                        return failure;
                        
                    } else if (compareTimes(endTime, db_start_time) == 1 && (compareTimes(endTime, db_end_time) == -1 || compareTimes(endTime, db_end_time) == 0)) {
                        // End time runs into another appointment
                        strcat(result, "ERROR: Your end time conflicts with another appointment");
                        strcat(result, "\n\tOld Appointment: ");
                        strcat(result, db_date);
                        strcat(result, " ");
                        strcat(result, db_start_time);
                        strcat(result, " - ");
                        strcat(result, db_end_time);
                        strcat(result, "\n\tNew Appointment: ");
                        strcat(result, date);
                        strcat(result, " ");
                        strcat(result, startTime);
                        strcat(result, " - ");
                        strcat(result, endTime);
                        char* failure = result;
                        return failure;
                        
                    } else if ((compareTimes(startTime, db_start_time) == -1 || compareTimes(startTime, db_start_time) == 0) && (compareTimes(endTime, db_end_time) == 1 || compareTimes(endTime, db_end_time) == 0)) {
                        // Appointment wraps another appointment
                        strcat(result, "ERROR: There is another appointment during this time already");
                        strcat(result, "\n\tOld Appointment: ");
                        strcat(result, db_date);
                        strcat(result, " ");
                        strcat(result, db_start_time);
                        strcat(result, " - ");
                        strcat(result, db_end_time);
                        strcat(result, "\n\tNew Appointment: ");
                        strcat(result, date);
                        strcat(result, " ");
                        strcat(result, startTime);
                        strcat(result, " - ");
                        strcat(result, endTime);
                        char* failure = result;
                        return failure;
                    }
                }
            }
        }
    }
    fclose(currentAppointmentsFile);
    
    // Create the new appointment
    FILE* oldAppointmentsFile = fopen("Appointment.csv", "r");
    FILE* newAppointmentsFile = fopen("newAppointment.csv", "w+");
    
    char oldAppointmentLine[1024];
    
    // Copy all of the information from old users file to the new users file
    while (fgets(oldAppointmentLine, sizeof(oldAppointmentLine), oldAppointmentsFile)) {
        char* db_entry = strdup(oldAppointmentLine);
        char* db_id = strtok(db_entry, ",");
        
        if (strcmp(id, db_id) == 0) {
            // Add in the new user
            fprintf(newAppointmentsFile, "%s,%s,%s,%s,%s,%s\n", id, key, name, date, startTime, endTime);
        } else {
            // Copy line
            fprintf(newAppointmentsFile, "%s", oldAppointmentLine);
            
        }
    }
    
    // Close the file
    fclose(oldAppointmentsFile);
    fclose(newAppointmentsFile);
    
    // Delete the old users file
    if (remove("Appointment.csv") != 0) {
        strcat(result, "ERROR: Edit unsuccessful. Problem deleting old file.");
        char* failure = result;
        return failure;
    }
    
    // Rename the new users file
    if (rename("newAppointment.csv", "Appointment.csv") != 0) {
        strcat(result, "ERROR: Edit unsuccessful. Problem saving");
        char* failure = result;
        return failure;
    }
    
    strcat(result, "Success: Your appointmnt has been edited");
    char* success = result;
    return success;

}

/**
 Get Event By Time
 
 Get an event based on a certain time
 */
char* getEventByTime(char* key, char* password, char* date, char* time) {
    
    char result[1024];
    
    // Check to see if user has correct password
    FILE* currentUserFile = fopen("User.csv", "r");
    char currentLine[1024];
    while (fgets(currentLine, sizeof(currentLine), currentUserFile)) {
        char* db_entry = strdup(currentLine);
        char* db_id = strtok(db_entry, ",");
        strtok(NULL, ","); // omit the email
        char* db_password = strtok(NULL, ",");
        
        if (strcmp(db_id, key) == 0) {
            if (strcmp(db_password, password) != 0) {
                strcat(result, "ERROR: Query unsuccessful. The password does not match for this user id");
                char* failure = result;
                return failure;
            }
            break;
        }
    }
    fclose(currentUserFile);
    
    char tempDate[1024];
    strcpy(tempDate, date);
    char tempTime[1024];
    strcpy(tempTime, time);
    
    // Validate Date
    if (!isValidDate(tempDate)) {
        strcat(result, "ERROR: The date entered is not valid. Please make sure the date is in the form: MM-DD-YYYY");
        char* failure = result;
        return failure;
    }
    
    // Validate Start Time
    if (!isValidTime(tempTime)) {
        strcat(result, "ERROR: The time entered is not valid. Please make sure the time is in the form: HH:MM, but DO NOT start with a '0'. (For example, 09:30 should be entered as 9:30, not 09:30)");
        char* failure = result;
        return failure;
    }

    FILE* appointmentsFile = fopen("Appointment.csv", "r");
    char currentAppointmentLine[1024];
    
    while (fgets(currentAppointmentLine, sizeof(currentAppointmentLine), appointmentsFile)) {
        
        char* db_entry = strdup(currentAppointmentLine);
        
        char* db_id = strtok(db_entry, ",");
        char* db_user_id = strtok(NULL, ",");
        
        if (strcmp(key, db_user_id) == 0) {
            // Same user
            
            char* db_name = strtok(NULL, ",");
            char* db_date = strtok(NULL, ",");
            
            if (strcmp(db_date, date) == 0) {
                // Same date
                
                char* db_start_time = strtok(NULL, ",");
                char* db_end_time = strtok(NULL, "");
                
                // Appointment is during the time asked for
                if ((compareTimes(time, db_start_time) == 1 || compareTimes(time, db_start_time) == 0) && (compareTimes(time, db_end_time) == -1 || compareTimes(time, db_end_time) == 0)) {
                    
                    strcat(result, "SUCCESS: Here is the appointment you have at the time requested:");
                    strcat(result, "\n\tID: ");
                    strcat(result, db_id);
                    strcat(result, " NAME: ");
                    strcat(result, db_name);
                    strcat(result, " DATE: ");
                    strcat(result, db_date);
                    strcat(result, " START TIME: ");
                    strcat(result, db_start_time);
                    strcat(result, " END TIME: ");
                    strcat(result, db_end_time);
                    char* success = result;
                    return success;
                }
            }
        }
    }
    
    strcat(result, "SUCCESS: You do not have any appointments at the time requested");
    char* success = result;
    return success;
}


/**
 Get Events By Time Range
 
 Gets all of the events within a time range
 */
char* getEventsByTimeRange(char* key, char* password, char* date, char* startTime, char* endTime) {
    
    char result[1024];
    
    // Check to see if user has correct password
    FILE* currentUserFile = fopen("User.csv", "r");
    char currentLine[1024];
    while (fgets(currentLine, sizeof(currentLine), currentUserFile)) {
        char* db_entry = strdup(currentLine);
        char* db_id = strtok(db_entry, ",");
        strtok(NULL, ","); // omit the email
        char* db_password = strtok(NULL, ",");
        
        if (strcmp(db_id, key) == 0) {
            if (strcmp(db_password, password) != 0) {
                strcat(result, "ERROR: Query unsuccessful. The password does not match for this user id");
                char* failure = result;
                return failure;
            }
            break;
        }
    }
    fclose(currentUserFile);
    
    char tempDate[1024];
    strcpy(tempDate, date);
    char tempStartTime[1024];
    strcpy(tempStartTime, startTime);
    char tempEndTime[1024];
    strcpy(tempEndTime, endTime);
    
    // Validate Start Date
    if (!isValidDate(tempDate)) {
        strcat(result, "ERROR: The start date entered is not valid. Please make sure the date is in the form: MM-DD-YYYY");
        char* failure = result;
        return failure;
    }
    
    // Validate Start Time
    if (!isValidTime(tempStartTime)) {
        strcat(result, "ERROR: The start time entered is not valid. Please make sure the time is in the form: HH:MM, but DO NOT start with a '0'. (For example, 09:30 should be entered as 9:30, not 09:30)");
        char* failure = result;
        return failure;
    }
    
    // Validate End Time
    if (!isValidTime(tempEndTime)) {
        strcat(result, "ERROR: The end time entered is not valid. Please make sure the time is in the form: HH:MM, but DO NOT start with a '0'. (For example, 09:30 should be entered as 9:30, not 09:30)");
        char* failure = result;
        return failure;
    }

    FILE* appointmentsFile = fopen("Appointment.csv", "r");
    char currentAppointmentLine[1024];
    
    bool foundOne = false;
    
    while (fgets(currentAppointmentLine, sizeof(currentAppointmentLine), appointmentsFile)) {
        
        char* db_entry = strdup(currentAppointmentLine);
        
        char* db_id = strtok(db_entry, ",");
        char* db_user_id = strtok(NULL, ",");
        
        if (strcmp(key, db_user_id) == 0) {
            // Same user
            
            char* db_name = strtok(NULL, ",");
            char* db_date = strtok(NULL, ",");

            if (strcmp(db_date, date) == 0) {
                // Event date falls between search dates
                
                char* db_start_time = strtok(NULL, ",");
                char* db_end_time = strtok(NULL, "");

                // Appointment is during the time asked for
                if ((compareTimes(db_start_time, startTime) == 1 || compareTimes(db_start_time, startTime) == 0 ) && (compareTimes(db_start_time, endTime) == -1 || compareTimes(db_start_time, endTime) == 0)) {
                    // Start time is in the range

                    if (!foundOne) {
                        strcat(result, "SUCCESS: Here are the appointments you have within that time range:\n");
                        foundOne = true;
                    }
                    
                    strcat(result, "\t");
                    strcat(result, db_id);
                    strcat(result, " ");
                    strcat(result, db_name);
                    strcat(result, " ");
                    strcat(result, db_date);
                    strcat(result, " ");
                    strcat(result, db_start_time);
                    strcat(result, " ");
                    strcat(result, db_end_time);
                    
                } else if ((compareTimes(db_end_time, startTime) == 1 || compareTimes(db_end_time, startTime) == 0 ) && (compareTimes(db_end_time, endTime) == -1 || compareTimes(db_end_time, endTime) == 0)) {
                    // End time is in the range
                    
                    if (!foundOne) {
                        strcat(result, "SUCCESS: Here are the appointments you have within that time range:\n");
                        foundOne = true;
                    }
                    
                    strcat(result, "\t");
                    strcat(result, db_id);
                    strcat(result, " ");
                    strcat(result, db_name);
                    strcat(result, " ");
                    strcat(result, db_date);
                    strcat(result, " ");
                    strcat(result, db_start_time);
                    strcat(result, " ");
                    strcat(result, db_end_time);
                }
                
                
            }
        }
    }
    
    if (!foundOne) {
        strcat(result, "SUCCESS: You do not have any appointments within the time range requested");
    }

    char* success = result;
    return success;
}

void sigchld_handler(int s) {
    while(wait(NULL) > 0);
}

int main(void) {
    
    int sockfd, new_fd;             // Listen on sock_fd, new connection on new_fd
    struct sockaddr_in my_addr;     // My address information
    struct sockaddr_in their_addr;  // Connector's address information
    int sin_size;
    struct sigaction sa;
    int yes = 1;
    char *recvbuf;
    char *caddr;
    int numbytes;
    
    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    
    printf("SOCK_FD = %d\n", sockfd);
    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("socket");
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;           // Host byte order
    my_addr.sin_port = htons(MYPORT);       // Short, network byte order
    my_addr.sin_addr.s_addr = INADDR_ANY;   // Automatically fill with my IP
    memset(&(my_addr.sin_zero), '\0', 8);   // Zero the rest of the struct
    
    // Bind the socket to a local IP address and port numner
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    
    // Put socket into passive state.
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    
    sa.sa_handler = sigchld_handler;     // Reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    
    while (1) {     // main accept() loop
        
        socklen_t sin_size = sizeof(struct sockaddr_in);

        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept");
            continue;
        }
        
        printf("server: got connection from %s\n", (char *) inet_ntoa(their_addr.sin_addr));
        
        if (!fork()) { // This is the child process
            close(sockfd);  // Child doesn't need the listener
            
            recvbuf = (char *) calloc(512, sizeof(char));
            
            for (;;) {
            
                // Recieve data from client
                
                numbytes = recv(new_fd, recvbuf, 512, 0);
                
                if (numbytes < 0) {
                    perror("recv");
                    close(new_fd);
                    exit(1);
                } else if (numbytes == 0 || strcmp(recvbuf, "exit") == 0) {
                    printf("Client (%s) has been disconnected\n", (char *) inet_ntoa(their_addr.sin_addr));
                    close(new_fd);
                    exit(0);
                }
                
                printf("Recieved from %s: %s\n", inet_ntoa(their_addr.sin_addr), recvbuf);
                
                // Begin actions
                
                strtok(recvbuf," ");
                
                char returnbuf[512] = "";
                
                if (strcmp(recvbuf, "login") == 0) {
                    
                    char * email = strtok(NULL," ");
                    char * password = strtok(NULL," ");
                    
                    // Process login
                    if (email == NULL || password == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to login as a user: (email, password)");
                    } else {
                        char* result = login(email, password);
                        strcat(returnbuf, result);
                    }
                    
                    
                } else if (strcmp(recvbuf, "logout") == 0) {
                    
                    char* id = strtok(NULL, " ");
                    
                    if (id == NULL) {
                        
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to logout a user: (key)");
                        
                    } else {
                        
                        if (logout(id)) {
                            strcat(returnbuf, "SUCCESS: User has been logged out");
                        } else {
                            strcat(returnbuf, "ERROR: Could not log user out");
                        }
                    }
                    
                } else if (strcmp(recvbuf, "add_account") == 0) {
                    
                    char* email = strtok(NULL, " ");
                    char* password = strtok(NULL, " ");
                    char* firstName = strtok(NULL, " ");
                    char* lastName = strtok(NULL, " ");
                    char* phone = strtok(NULL, " ");
                    
                    if (email == NULL || password == NULL || firstName == NULL || lastName == NULL || phone == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to create a user: (email, password, firstname, lastname, and phone number)");
                    } else {
                        char* result = createAccount(email, password, firstName, lastName, phone);
                        strcat(returnbuf, result);
                    }
                    
                } else if (strcmp(recvbuf, "delete_account") == 0) {
                    
                    char* id = strtok(NULL, " ");
                    char* password = strtok(NULL, " ");
                    
                    if (id == NULL || password == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to delete a user: (id, password)");
                    } else {
                        if (isLoggedIn(id)) {
                            char* result = deleteAccount(id, password);
                            strcat(returnbuf, result);
                        } else {
                            strcat(returnbuf, "ERROR: You need to be logged in to delete your account");
                        }
                    }
                    
                } else if (strcmp(recvbuf, "edit_account") == 0) {
                    
                    char* id = strtok(NULL, " ");
                    char* currentPassword = strtok(NULL, " ");
                    char* email = strtok(NULL, " ");
                    char* newPassword = strtok(NULL, " ");
                    char* firstName = strtok(NULL, " ");
                    char* lastName = strtok(NULL, " ");
                    char* phone = strtok(NULL, " ");
                    
                    if (id == NULL || currentPassword == NULL || email == NULL || newPassword == NULL || firstName == NULL || lastName == NULL || phone == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to delete a user: (id, current password, email, new password, firstname, lastname, phone)");
                    } else {
                        if (isLoggedIn(id)) {
                            char* result = editAccount(id, currentPassword, email, newPassword, firstName, lastName, phone);
                            strcat(returnbuf, result);
                        } else {
                            strcat(returnbuf, "ERROR: User is not logged in");
                        }
                    }

                    
                } else if (strcmp(recvbuf, "add_event") == 0) {
                    
                    char* userId = strtok(NULL, " ");
                    char* name = strtok(NULL, " ");
                    char* date = strtok(NULL, " ");
                    char* startTime = strtok(NULL, " ");
                    char* endTime = strtok(NULL, " ");
                    
                    if (userId == NULL || name == NULL || date == NULL || startTime == NULL || endTime == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to add en event: (key, appointment name, date, start time, and time)");
                    } else {
                        if (isLoggedIn(userId)) {
                            
                            char* result = createEvent(userId, name, date, startTime, endTime);
                            strcat(returnbuf, result);
                            
                        } else {
                            strcat(returnbuf, "ERROR: User is not logged in");
                        }
                    }
                    
                } else if (strcmp(recvbuf, "delete_event") == 0) {
                    
                    char* id = strtok(NULL, " ");
                    char* password = strtok(NULL, " ");
                    char* eventId = strtok(NULL, " ");
                    
                    if(id == NULL || eventId == NULL || password == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to add en event: (key, event id, password)");
                    } else {
                        if (isLoggedIn(id)) {
                            char* result = deleteEvent(eventId, id, password);
                            strcat(returnbuf, result);
                        } else {
                            strcat(returnbuf, "ERROR: User is not logged in");
                        }
                    }

                    
                } else if (strcmp(recvbuf, "edit_event") == 0) {
                    
                    char* id = strtok(NULL, " ");
                    char* password = strtok(NULL, " ");
                    char* eventId = strtok(NULL, " ");
                    char* name = strtok(NULL, " ");
                    char* date = strtok(NULL, " ");
                    char* startTime = strtok(NULL, " ");
                    char* endTime = strtok(NULL, " ");
                    
                    if (id == NULL || password == NULL || eventId == NULL || name == NULL || date == NULL || startTime == NULL || endTime == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to add en event: (key, password, event id, appointment name, date, start time, and time)");
                    } else {
                        if (isLoggedIn(id)) {
                            char* result = editEvent(eventId, id, password, name, date, startTime, endTime);
                            strcat(returnbuf, result);
                        } else {
                            strcat(returnbuf, "ERROR: User is not logged in");
                        }
                    }

                    
                } else if (strcmp(recvbuf, "event_by_time") == 0) {
                    
                    char* id = strtok(NULL, " ");
                    char* password = strtok(NULL, " ");
                    char* date = strtok(NULL, " ");
                    char* time = strtok(NULL, " ");
                    
                    if (id == NULL || password == NULL || date == NULL || time == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to add en event: (key, password, date, time)");
                    } else {
                        if (isLoggedIn(id)) {
                            char* result = getEventByTime(id, password, date, time);
                            strcat(returnbuf, result);
                        } else {
                            strcat(returnbuf, "ERROR: User is not logged in");
                        }
                    }
                    
                } else if (strcmp(recvbuf, "events_by_time_range") == 0) {
                    
                    char* id = strtok(NULL, " ");
                    char* password = strtok(NULL, " ");
                    char* date = strtok(NULL, " ");
                    char* startTime = strtok(NULL, " ");
                    char* endTime = strtok(NULL, " ");
                    
                    if (id == NULL || password == NULL || date == NULL || startTime == NULL || endTime == NULL) {
                        strcat(returnbuf, "ERROR: You are missing some information. Please provide all the information to add en event: (key, password, start date, start time, end date, end time)");
                    } else {
                        if (isLoggedIn(id)) {
                            char* result = getEventsByTimeRange(id, password, date, startTime, endTime);
                            strcat(returnbuf, result);
                        } else {
                            strcat(returnbuf, "ERROR: User is not logged in");
                        }
                    }
                    
                } else {
                    strcat(returnbuf, "Action not recognized");
                }

                if (send(new_fd, returnbuf, sizeof(returnbuf), 0) == -1) {
                    perror("send");
                    close(new_fd);
                    close(1);
                }
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }
    return 0;
}
