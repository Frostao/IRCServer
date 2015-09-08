#include "HashTableVoid.h"

#ifndef IRC_SERVER
#define IRC_SERVER

#define PASSWORD_FILE "password.txt"
struct Message {
    char * user;
    char * message;
};

struct ChatRoom {
    char * name;
    int amountOfUsers;
    char *users [1000];
    int amountOfMessages;
    Message messages [1000];
};




class IRCServer {
	// Add any variables you need
    
private:
	int open_server_socket(int port);

public:
	void initialize();
	bool checkPassword(int fd, const char * user, const char * password);
	void processRequest( int socket );
	void addUser(int fd, const char * user, const char * password, const char * args);
	void enterRoom(int fd, const char * user, const char * password, const char * args);
    void createRoom(int fd, const char * user, const char * password, const char * args);
    void listRooms(int fd, const char * user, const char * password, const char * args);
	void leaveRoom(int fd, const char * user, const char * password, const char * args);
	void sendMessage(int fd, const char * user, const char * password, const char * args);
	void getMessages(int fd, const char * user, const char * password, const char * args);
	void getUsersInRoom(int fd, const char * user, const char * password, const char * args);
	void getAllUsers(int fd, const char * user, const char * password, const char * args);
	void runServer(int port);
};

#endif