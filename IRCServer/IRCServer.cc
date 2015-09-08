
const char * usage =
"                                                               \n"
"IRCServer:                                                   \n"
"                                                               \n"
"Simple server program used to communicate multiple users       \n"
"                                                               \n"
"To use it in one window type:                                  \n"
"                                                               \n"
"   IRCServer <port>                                          \n"
"                                                               \n"
"Where 1024 < port < 65536.                                     \n"
"                                                               \n"
"In another window type:                                        \n"
"                                                               \n"
"   telnet <host> <port>                                        \n"
"                                                               \n"
"where <host> is the name of the machine where talk-server      \n"
"is running. <port> is the port number you used when you run    \n"
"daytime-server.                                                \n"
"                                                               \n";

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "IRCServer.h"


int QueueLength = 5;
HashTableVoid users;

ChatRoom chatRooms [100];
int chatRoomIndex = 0;

int
IRCServer::open_server_socket(int port) {

	// Set the IP address and port for this server
	struct sockaddr_in serverIPAddress; 
	memset( &serverIPAddress, 0, sizeof(serverIPAddress) );
	serverIPAddress.sin_family = AF_INET;
	serverIPAddress.sin_addr.s_addr = INADDR_ANY;
	serverIPAddress.sin_port = htons((u_short) port);
  
	// Allocate a socket
	int masterSocket =  socket(PF_INET, SOCK_STREAM, 0);
	if ( masterSocket < 0) {
		perror("socket");
		exit( -1 );
	}

	// Set socket options to reuse port. Otherwise we will
	// have to wait about 2 minutes before reusing the sae port number
	int optval = 1; 
	int err = setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, 
			     (char *) &optval, sizeof( int ) );
	
	// Bind the socket to the IP address and port
	int error = bind( masterSocket,
			  (struct sockaddr *)&serverIPAddress,
			  sizeof(serverIPAddress) );
	if ( error ) {
		perror("bind");
		exit( -1 );
	}
	
	// Put socket in listening mode and set the 
	// size of the queue of unprocessed connections
	error = listen( masterSocket, QueueLength);
	if ( error ) {
		perror("listen");
		exit( -1 );
	}

	return masterSocket;
}

void
IRCServer::runServer(int port)
{
	int masterSocket = open_server_socket(port);

	initialize();
	
	while ( 1 ) {
		
		// Accept incoming connections
		struct sockaddr_in clientIPAddress;
		int alen = sizeof( clientIPAddress );
		int slaveSocket = accept( masterSocket,
					  (struct sockaddr *)&clientIPAddress,
					  (socklen_t*)&alen);
		
		if ( slaveSocket < 0 ) {
			perror( "accept" );
			exit( -1 );
		}
		
		// Process request.
		processRequest( slaveSocket );		
	}
}

int
main( int argc, char ** argv )
{
	// Print usage if not enough arguments
	if ( argc < 2 ) {
		fprintf( stderr, "%s", usage );
		exit( -1 );
	}
	
	// Get the port from the arguments
	int port = atoi( argv[1] );

	IRCServer ircServer;

	// It will never return
	ircServer.runServer(port);
	
}

//
// Commands:
//   Commands are started y the client.
//
//   Request: ADD-USER <USER> <PASSWD>\r\n
//   Answer: OK\r\n or DENIED\r\n
//
//   REQUEST: GET-ALL-USERS <USER> <PASSWD>\r\n
//   Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//
//   REQUEST: CREATE-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LIST-ROOMS <USER> <PASSWD>\r\n
//   Answer: room1\r\n
//           room2\r\n
//           ...
//           \r\n
//
//   Request: ENTER-ROOM <USER> <PASSWD> <ROOM>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: LEAVE-ROOM <USER> <PASSWD>\r\n
//   Answer: OK\n or DENIED\r\n
//
//   Request: SEND-MESSAGE <USER> <PASSWD> <MESSAGE> <ROOM>\n
//   Answer: OK\n or DENIED\n
//
//   Request: GET-MESSAGES <USER> <PASSWD> <LAST-MESSAGE-NUM> <ROOM>\r\n
//   Answer: MSGNUM1 USER1 MESSAGE1\r\n
//           MSGNUM2 USER2 MESSAGE2\r\n
//           MSGNUM3 USER2 MESSAGE2\r\n
//           ...\r\n
//           \r\n
//
//    REQUEST: GET-USERS-IN-ROOM <USER> <PASSWD> <ROOM>\r\n
//    Answer: USER1\r\n
//            USER2\r\n
//            ...
//            \r\n
//

void
IRCServer::processRequest( int fd )
{
	// Buffer used to store the comand received from the client
	const int MaxCommandLine = 1024;
	char commandLine[ MaxCommandLine + 1 ];
	int commandLineLength = 0;
	int n;
	
	// Currently character read
	unsigned char prevChar = 0;
	unsigned char newChar = 0;
	
	//
	// The client should send COMMAND-LINE\n
	// Read the name of the client character by character until a
	// \n is found.
	//

	// Read character by character until a \n is found or the command string is full.
	while ( commandLineLength < MaxCommandLine &&
		read( fd, &newChar, 1) > 0 ) {

		if (newChar == '\n' && prevChar == '\r') {
			break;
		}
		
		commandLine[ commandLineLength ] = newChar;
		commandLineLength++;

		prevChar = newChar;
	}
	
	// Add null character at the end of the string
	// Eliminate last \r
	commandLineLength--;
        commandLine[ commandLineLength ] = 0;

	printf("RECEIVED: %s\n", commandLine);

	const char * command = strtok(commandLine, " ");
	const char * user = strtok(NULL, " ");
	const char * password = strtok(NULL, " ");;
	const char * args = strtok(NULL,"");

//	printf("command=%s\n", command);
//	printf("user=%s\n", user);
//	printf( "password=%s\n", password );
//	printf("args=%s\n", args);

	if (!strcmp(command, "ADD-USER")) {
		addUser(fd, user, password, args);
	} else if (!strcmp(command, "LEAVE-ROOM")) {
		leaveRoom(fd, user, password, args);
	} else if (!strcmp(command, "CREATE-ROOM")) {
        createRoom(fd, user, password, args);
    } else if (!strcmp(command, "LIST-ROOMS")) {
        listRooms(fd, user, password, args);
    } else if (!strcmp(command, "ENTER-ROOM")) {
        enterRoom(fd, user, password, args);
    } else if (!strcmp(command, "SEND-MESSAGE")) {
		sendMessage(fd, user, password, args);
	} else if (!strcmp(command, "GET-MESSAGES")) {
		getMessages(fd, user, password, args);
	} else if (!strcmp(command, "GET-USERS-IN-ROOM")) {
		getUsersInRoom(fd, user, password, args);
	} else if (!strcmp(command, "GET-ALL-USERS")) {
		getAllUsers(fd, user, password, args);
	} else {
		const char * msg =  "UNKNOWN COMMAND\r\n";
		write(fd, msg, strlen(msg));
	}

	// Send OK answer
	//const char * msg =  "OK\n";
	//write(fd, msg, strlen(msg));

	close(fd);
}

int findRoomIndexByName(const char * name) {
    int i = 0;
    if (chatRooms[i].name == NULL) {
        return -1;
    }
    while (strcmp(chatRooms[i].name, name) != 0) {
        
        i++;
        if (i == chatRoomIndex) {
            //there is no room matched that name
            return -1;
        }
    }
    //printf("i=%d, chatroomindex = %d",i,chatRoomIndex);
    return i;
}

void
IRCServer::initialize()
{
	// Open password file
    FILE *f = fopen(PASSWORD_FILE, "a+");
    
	// Initialize users in room
    char username [100]; //Max length username
    char password [100]; // max length password
    while (fscanf(f,"%s %s\n", username, password) != EOF) {
        users.insertItem(username, (void*)strdup(password));
//        HashTableVoidIterator iterator(&users);
//        const char * key;
//        void * data;
//        while (iterator.next(key, data)) {
//            printf("%s %s\n", key, (char*)data);
//        }
        //printf("%s %s\n", username, password);
    }
	// Initalize message list
    fclose(f);
}

bool
IRCServer::checkPassword(int fd, const char * user, const char * password) {
	// Here check the password
    void * data;
    if (user == NULL || password == NULL) {
        return false;
    }
    
    
    if (users.find(user, &data)) {
    	//printf("%s %s\n",user, (char*) data );
        if (!strcmp((char*)data, password)) {
            return true;
        }
    }
	return false;
}

void IRCServer::addUser(int fd, const char * user, const char * password, const char * args) {
	// Here add a new user. For now always return OK.
	void *data;
	if (users.find(user, &data))
	{
		const char * msg =  "User exist\r\n";
		write(fd, msg, strlen(msg));
	} else {
		FILE *f = fopen(PASSWORD_FILE, "a");
		fprintf(f,"%s %s\n",user,password);
		users.insertItem(user, (void*)strdup(password));
		const char * msg =  "OK\r\n";
		write(fd, msg, strlen(msg));
		fclose(f);
    }	
}

void IRCServer::enterRoom(int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password)) {
        int roomIndex = findRoomIndexByName(args);
        if (roomIndex == -1) {
            write(fd, "ERROR (No room)\r\n", strlen("ERROR (No room)\r\n"));
            return;
        }
        int amountOfUsers = chatRooms[roomIndex].amountOfUsers;
        bool inTheRoom = false;
        for (int i = 0; i < amountOfUsers && chatRooms[roomIndex].users[i] != NULL; i++) {
            //printf("user in room is %s, compare to %s\n", chatRooms[roomIndex].users[i],user);
            if (!strcmp(chatRooms[roomIndex].users[i],user)) {
                //found it
                inTheRoom = true;
            }
        }
        if (inTheRoom){
            write(fd, "OK\r\n", strlen("OK\r\n"));
            return;
        }
        chatRooms[roomIndex].users[amountOfUsers] = strdup(user);
        chatRooms[roomIndex].amountOfUsers++;
        char msg [100] = "OK\r\n";
        char * username = chatRooms[roomIndex].users[amountOfUsers];
        //sprintf(msg, "Entered room %d, name = %s\n",roomIndex,username);
        write(fd, msg, strlen(msg));
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
    
}



void IRCServer::createRoom(int fd, const char * user, const char * password, const char * args) {
    
    if (checkPassword(fd, user, password)) {
        chatRooms[chatRoomIndex].name = strdup(args);
        chatRooms[chatRoomIndex].amountOfUsers = 0;
        chatRoomIndex++;
        char msg [100] = "OK\r\n";
        //sprintf(msg, "%s + %d", args, chatRoomIndex);
        write(fd, msg, strlen(msg));
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
    
    return;
}
void IRCServer::listRooms(int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password)) {
        int i = 0;
        while (chatRooms[i].name != NULL) {
            char msg [100];
            sprintf(msg, "%d=%s\n",i,chatRooms[i].name);
            write(fd, msg, strlen(msg));
            i++;
        }
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
    
    
    return;
}




void IRCServer::leaveRoom(int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password)) {
        int roomIndex = findRoomIndexByName(args);
        if (roomIndex == -1) {
        	write(fd, "ERROR (No room)\r\n", strlen("ERROR (No room)\r\n"));
        	return;
        }
        int amountOfUsers = chatRooms[roomIndex].amountOfUsers;
        int i = 0;
        for (; i < amountOfUsers && chatRooms[roomIndex].users[i] != NULL; i++) {
        	//printf("user in room is %s, compare to %s\n", chatRooms[roomIndex].users[i],user);
            if (!strcmp(chatRooms[roomIndex].users[i],user)) {
            	for (int j = i; j < amountOfUsers; j++) {
            		chatRooms[roomIndex].users[j] = chatRooms[roomIndex].users[j+1];
            	}
                //chatRooms[roomIndex].users[i] = NULL;
                chatRooms[roomIndex].amountOfUsers--;
            }
        }
        if (amountOfUsers > chatRooms[roomIndex].amountOfUsers) {
            char msg [100];
            char * username = chatRooms[roomIndex].users[i];
            sprintf(msg, "OK\r\n",roomIndex,username);
            write(fd, msg, strlen(msg));
        } else {
            write(fd, "ERROR (No user in room)\r\n", strlen("ERROR (No user in room)\r\n"));
        }
        
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
    
}

void IRCServer::sendMessage(int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password)) {
        char * room = strtok((char*)args, " ");
        char * message = strtok(NULL, "");
//        char msg[100];
//        sprintf(msg, "room = %s, message = %s\n", room, message);
//        write(fd, msg, strlen(msg));
        int roomIndex = findRoomIndexByName(room);
        //check if user is in the room
        int amountOfUsers = chatRooms[roomIndex].amountOfUsers;
        bool inTheRoom = false;
        for (int i = 0; i < amountOfUsers && chatRooms[roomIndex].users[i] != NULL; i++) {
            //printf("user in room is %s, compare to %s\n", chatRooms[roomIndex].users[i],user);
            if (!strcmp(chatRooms[roomIndex].users[i],user)) {
                //found it
                inTheRoom = true;
            }
        }
        if (!inTheRoom){
            write(fd, "ERROR (user not in room)\r\n", strlen("ERROR (user not in room)\r\n"));
            return;
        }

        if (roomIndex != -1) {
            int amountOfMessages = chatRooms[roomIndex].amountOfMessages;
            chatRooms[roomIndex].messages[amountOfMessages].user = strdup(user);
            chatRooms[roomIndex].messages[amountOfMessages].message = strdup(message);
            chatRooms[roomIndex].amountOfMessages++;
            const char * amsg = "OK\r\n";
            write(fd, amsg, strlen(amsg));
            return;
        } else {
            const char * amsg = "room not found!\r\n";
            write(fd, amsg, strlen(amsg));
        }
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
}

void IRCServer::getMessages(int fd, const char * user, const char * password, const char * args) {
    if (args != NULL && checkPassword(fd, user, password) ) {
        int lastMessageNum = atoi(strtok((char*)args, " "));
        char * room = strtok(NULL, " ");
        int roomIndex = findRoomIndexByName(room);
        int amountOfMessages = chatRooms[roomIndex].amountOfMessages;
        int amountOfUsers = chatRooms[roomIndex].amountOfUsers;
        bool inTheRoom = false;
        for (int i = 0; i < amountOfUsers && chatRooms[roomIndex].users[i] != NULL; i++) {
            //printf("user in room is %s, compare to %s\n", chatRooms[roomIndex].users[i],user);
            if (!strcmp(chatRooms[roomIndex].users[i],user)) {
                //found it
                inTheRoom = true;
            }
        }
        if (!inTheRoom){
            write(fd, "ERROR (User not in room)\r\n", strlen("ERROR (User not in room)\r\n"));
            return;
        }
        
//        char msg[100];
//        sprintf(msg, "room = %s, lastmsgnum = %d\n", room, lastMessageNum);
//        write(fd, msg, strlen(msg));
        if (lastMessageNum >= amountOfMessages) {
            lastMessageNum = amountOfMessages;
            write(fd, "NO-NEW-MESSAGES\r\n", strlen("NO-NEW-MESSAGES\r\n"));
            return;
        }
//        } else if (lastMessageNum+1 == amountOfMessages) {
//            write(fd, "NO-NEW-MESSAGES\r\n", strlen("NO-NEW-MESSAGES\r\n"));
//            return;
//        }
        for (int i = lastMessageNum; i < amountOfMessages; i++) {
            char msg[100];
            sprintf(msg, "%d %s %s\r\n",i,chatRooms[roomIndex].messages[i].user, chatRooms[roomIndex].messages[i].message);
            write(fd, msg, strlen(msg));
        }
        write(fd, "\r\n", strlen("\r\n"));
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
}
void sortUsernames(char ** & usernames, int length) {
	int flag = 1;
    for (int i = 1; (i <= length)&&flag; i++) {
        flag = 0;
        for (int j = 0; j < length-1; j++)
        {
            if (strcmp(usernames[j],usernames[j+1])>0) {
                char * a = usernames[j];
                usernames[j] = usernames[j+1];
                usernames[j+1] = a;
                flag = 1;
            }
            
        }
        
    }
}
void IRCServer::getUsersInRoom(int fd, const char * user, const char * password, const char * args) {
    if (checkPassword(fd, user, password)) {
        int roomIndex = findRoomIndexByName(args);
        if (roomIndex == -1) {
            write(fd, "ERROR (No room)\r\n", strlen("ERROR (No room)\r\n"));
            return;
        }
        int amountOfUsers = chatRooms[roomIndex].amountOfUsers;
        int count = 0;
        int i = 0;
        char ** usernames = (char**) malloc(100*sizeof(char*)*100);
        //int length = 0;
        while (1) {
            if (chatRooms[roomIndex].users[i] != NULL) {
                //char msg [100];
                //sprintf(msg, "chatroomIndex = %d, %d=%s\n", roomIndex, i, chatRooms[roomIndex].users[i]);
                //sprintf(msg, "%s\r\n",chatRooms[roomIndex].users[i]);
                //write(fd,msg,strlen(msg));
                usernames[count] = chatRooms[roomIndex].users[i];
                count++;
            }
            i++;
            if (count == amountOfUsers) {
                break;
            }
        }
        sortUsernames(usernames, count);
        for (int i = 0; i < count; i++) {
        	char msg [100];
            sprintf(msg, "%s\r\n",usernames[i]);
            write(fd, msg, strlen(msg));
        }
        write(fd, "\r\n", strlen("\r\n"));
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
    
}

void IRCServer::getAllUsers(int fd, const char * user, const char * password,const  char * args) {
    //works
    if (checkPassword(fd, user, password)) {
        HashTableVoidIterator iterator(&users);
        const char * username;
        void * pass;
        char ** usernames = (char**) malloc(100*sizeof(char*)*100);
        int length = 0;
        while (iterator.next(username,pass)) {
        	usernames[length] = strdup(username);
        	length++;
            // char msg [100];
            // sprintf(msg, "%s\r\n",username);
            // write(fd, msg, strlen(msg));
        }
        sortUsernames(usernames, length);
        for (int i = 0; i < length; i++) {
        	char msg [100];
            sprintf(msg, "%s\r\n",usernames[i]);
            write(fd, msg, strlen(msg));
        }
        write(fd, "\r\n", strlen("\r\n"));
    } else {
        write(fd, "ERROR (Wrong password)\r\n", strlen("ERROR (Wrong password)\r\n"));
    }
    
}



