/**********************************************
 * Last Name:   Syed
 * First Name:  Masroor Hussain
 * Student ID:  30023900
 * Course:      CPSC 441
 * Tutorial:    T-05
 * Assignment:  1
 * Description: A HTTP proxy which randomly change some of the text 
 *              or HTML content before it is delivered to the browser.
 *
 * File name: proxy.cpp
 *********************************************/

#include <stdio.h>
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <ctime>

#define port 15549    // port at the which the proxy runs at 

using namespace std;

string get_hname(string req);
string proxy_client(string hname, string user_req);
int get_server_res_code(string ser_res);
string get_content_type(string res);
string rand_str();



char req_buff[1024*1024];   // 1MB buff to temp hold the recived data from connections
char other_buff[1024*1024]; // 1mb buff to store binary server return val

int main(int argc, char const *argv[])
{
   
    memset(&req_buff,'\0',sizeof(req_buff)); // clear the buffer

	//Address initialization of the proxy server that the browser talks to 
	struct sockaddr_in s_address;
	memset(&s_address,0,sizeof(s_address));
	s_address.sin_family = AF_INET; //Address family
	s_address.sin_port = htons(port);
	s_address.sin_addr.s_addr = htonl(INADDR_ANY); // any ip on local host

	//Socket creation
    int listening_socket; //listening socket , file descriptor
	listening_socket= socket(AF_INET,SOCK_STREAM,0); 
	if(listening_socket==-1) 
    {
        cout << "Socket creation failed!!\n";
        exit(-1);
    }
    cout << "Socket created\n";

	//Binding 
    int bind_status;
	bind_status = bind(listening_socket, (struct sockaddr *)&s_address, sizeof(s_address));
	if(bind_status==-1)
    {
        cout << "Bind() failed ! \n";
        close(listening_socket);
        exit(-1);
    }
    cout <<"Bind created\n";

    // Listening
    int lis_status;
	lis_status = listen(listening_socket,3);//1 connections can listen
	if(lis_status<0)
    {
        cout << "Listen() Failed !\n";
        close(listening_socket);
        exit(-1);
    }
    cout <<"Listening now \n";

    while(1) 
    {   
        // accept a connection from the first client(web browser) in the queqe
        int msger_socket;   // fd for a messenger between the proxy and the client (used to send and recieve data from the client because listening socket is a passive listiner)
        msger_socket = accept( listening_socket, NULL, NULL);
        if( msger_socket == -1 ) 
        {
            cout << "Accept() Failed !\n";
            exit(-1);
        }
        cout << "Connection accepted now && a messenger socket Created!!!\n";

        /* Receive data from the browser */
        string user_req = "";    // buff to hold http requests
        int req_recieved;
        string ser_res = "";
       
        req_recieved = recv(msger_socket, req_buff, sizeof(req_buff),0) ;
        if (req_recieved < 0) 
        {
            cout << "Erorr with reading req from browers\n";
            exit(-1);
        } 
        user_req += req_buff;
        memset(&req_buff,'\0',sizeof(req_buff)); // free the buffer

        //memset(&req_buff,'\0',sizeof(req_buff)); // free the buffer
        cout << "PRINTING USER REQ: \n";
        cout << "Server Read Msg :\n" << user_req <<endl;

        cout << "\n-------------REQ PARSING!!!--------------\n";

        // get the req type 
        string req_type = user_req.substr(0,user_req.find(" "));

        if(req_type.compare("GET") != 0) {
            cout << "\n-------------CAN ONLY HANDLE GET REQUEST!!!--------------\n";
            cout << "\n-------------HAHA HAHAHA HAHAHHAHAHA HA!!!--------------\n";
            continue;
        }
        // get the url
        string url_s = user_req.substr(user_req.find(" ")+1, user_req.substr(user_req.find(" ")+1).find(" "));
        // get the host name from the url
        string hname = get_hname(url_s);
        cout << "\n-------------[got the req_type,url,hname]--------------\n";
        cout << "req_type: " << req_type << "\n" << "url: " << url_s << "\n" << "hostname: " << hname <<endl;

        // get the msg from the user 
        ser_res = proxy_client(hname,user_req);

        int res_code = get_server_res_code(ser_res);

        // if the response code is >= 300 then directly send to browser
        if (res_code >= 300) 
        {
            int s_status;
            s_status = send(msger_socket,ser_res.c_str(),strlen(ser_res.c_str()),0);
            if(s_status < 0) 
            {
                cout << "Failed to send server data to the browser!\n";
                exit(-1);
            }
            printf("Msg sent to browser \n");
        }else 
        {
            //  DO SOME MAGIC HERE!
           
            srand(time(NULL));

            //  get the content type
            string cnt_type = get_content_type(ser_res);
            cout << "Content-Type: " << cnt_type <<endl;

            //  deals with text
            if (cnt_type.compare("text/plain") == 0) 
            {
                cout << "\n--------------------------------\n";   
                cout << "FOUND TEXT!!\n";
                cout << "\n--------------------------------\n";   

                char blank = 32;
                string new_serres = ser_res;
                // mess up loop
                int bodys = ser_res.find("Content-Type") + ser_res.substr(ser_res.find("Content-Type")).find("\n")+1;
                string body = ser_res.substr(bodys);
                cout << "BODY!!!\n" << body;
                int blen = body.length();

                // insert the error in the text at the random place in th text
                for(int i = 0; i < 10; i++)
                {
                    int irand =  (rand()%blen) + bodys;
                    cout << "Body start index: " << bodys <<endl;
                    cout << "random index : " << irand <<endl;
                    cout << "body len : " << blen <<endl;
                    cout <<  "ser_res length: " <<  ser_res.length() << endl;
                    string rs = rand_str();
                    ser_res.insert(irand,rs);

                }
                // change content length
                int cntleni = ser_res.find("Content-Length:")+ strlen("Content-Length: ");
                string cntl = ser_res.substr(cntleni,ser_res.substr(ser_res.find("Content-Length: ")).find("\n"));
                int scntl = cntl.length();
                // earse the cnt len
                ser_res.erase(cntleni,scntl);
                
                int newlen = stoi(cntl) + (9*10); // adding 2 str and 7 for bold 
                cout << "CONTENT LENGTH :" << cntl <<endl;
                cout << "NEW CONTENT LENGTH :" << newlen <<endl;
                ser_res.insert(cntleni,to_string(newlen));

                // change viewing to html
                int cnti = ser_res.find("Content-Type")+14;
                ser_res = ser_res.erase(cnti,10);
                ser_res = ser_res.insert(cnti,"text/html");

            }
            //  deals with html
            else if (cnt_type.compare("text/html") == 0) 
            {
                cout << "\n--------------------------------\n";   
                cout << "HTML FOUND!!!\n";
                cout << "\n--------------------------------\n";  

                char blank = 32;
                string new_serres = ser_res;
                // mess up loop
                int bodys = ser_res.find("Content-Type") + ser_res.substr(ser_res.find("Content-Type")).find("\n")+1;
                string body = ser_res.substr(bodys);
                cout << "BODY!!!\n" << body;
                int blen = body.length();
                // insert 10 random things!!!
                for(int i = 0; i < 10; i++)
                {
                    int irand =  (rand()%blen) + bodys; // should be body len
                    cout << "random index : " << irand <<endl;
                    cout << "body len : " << blen <<endl;
                    cout <<  "ser_res length: " <<  ser_res.length() << endl;
                    string rs = rand_str();
                    ser_res.insert(irand,rs);

                }

                // change content length
                int cntleni = ser_res.find("Content-Length:")+ strlen("Content-Length: ");
                string cntl = ser_res.substr(cntleni,ser_res.substr(ser_res.find("Content-Length: ")).find("\n"));
                int scntl = cntl.length();
                // earse the cnt len
                ser_res.erase(cntleni,scntl);
                
                int newlen = stoi(cntl) + (9*10); // adding 2 str and 7 for bold 
                cout << "CONTENT LENGTH :" << cntl <<endl;
                cout << "NEW CONTENT LENGTH :" << newlen <<endl;
                ser_res.insert(cntleni,to_string(newlen));
                
            }
            else 
            {
                // if its a gif or image and just dont modify it just send it!!!
                int s_status;
                s_status = send(msger_socket,other_buff,sizeof(other_buff),0);
                if(s_status < 0) 
                {
                    cout << "Failed to send server data to the browser!\n";
                    exit(-1);
                }
                printf("Msg sent to browser \n");
                cout << ser_res <<endl;
                continue; // just continue the loop for gif
            }

            // send the msg back to browser
            int s_status;
            s_status = send(msger_socket,ser_res.c_str(),strlen(ser_res.c_str()),0);
            if(s_status < 0) 
            {
                cout << "Failed to send server data to the browser!\n";
                exit(-1);
            }
            printf("Msg sent to browser \n");
            cout << ser_res <<endl;
        }

        cout << "\n--------------------------------\n";        
        cout << "Closing the initial connection"<<endl;
        cout << "\n--------------------------------\n"; 
        close(msger_socket);
    }

    close(listening_socket);
	return 0;

}


// handles the connection between our proxy and the webserver
string proxy_client(string hname,string user_req)
{
    cout << "Now in proxy_client\n--------------------------------\n";
    int http_port = 80;

    //Address initialization	
    struct sockaddr_in web_server_address;
    bzero((char *) &web_server_address, sizeof(web_server_address));
    web_server_address.sin_family = AF_INET; //Address family
    struct hostent *address;
    address = gethostbyname(hname.c_str());
    bcopy( (char *) address->h_addr, (char *) &web_server_address.sin_addr.s_addr,address->h_length );
    web_server_address.sin_port = htons(http_port);
    
    //Socket creation
    int proxy_socket; //listening socket , file descriptor
	proxy_socket= socket(AF_INET,SOCK_STREAM,0); 
	if(proxy_socket==-1) cout <<"Socket creation failed!!\n";
    cout << "Socket created\n";


    // Connection Request
	int conn_status;
	conn_status = connect(proxy_socket,(struct sockaddr *)&web_server_address,sizeof(struct sockaddr_in));
	if(conn_status<0)
    {
        cout << "connect failed !!\n";
        exit(-1);
    }
    cout <<"connection made with the browser!!! \n";

    // send the http request
    int sent_status;
    sent_status = send(proxy_socket,user_req.c_str(),strlen(user_req.c_str()),0);
	if (sent_status < 0)
    {
        cout << "ERROR IN SENDING REQUEST TO THE SERVER\n";
        exit(-1);
    }
    cout << "Msg sent to server \n";

    // reading data from the webserver
    string ser_data = "";
    int read_status;
    read_status = recv(proxy_socket,req_buff,sizeof(req_buff),0);
    if (read_status < 0) 
    {
        cout << "Erorr with reading req from web server!\n";
        exit(-1);
    }

    // also copy the res in global var other buff
    memset(&other_buff,'\0',sizeof(other_buff));
    memcpy(&other_buff,&req_buff,read_status);
    //d s size

    ser_data += req_buff;
    memset(&req_buff,'\0',sizeof(req_buff)); // free the buffer
    
    cout << "\n--------------------------------\n";
    cout << "Received Msg";
    cout << "\n--------------------------------\n";
    cout << ser_data <<endl;

    close(proxy_socket);

    return ser_data;
    

}

// get the host name given url
string get_hname(string req)
{
    string hname;
    int hs = req.find('/') + 2;
    int he = req.substr(hs).find('/');
    hname = req.substr(hs,he);

    return hname;
}

// get the resonse code given server response
int get_server_res_code(string ser_res) 
{
    // get the first line of the ser_res
    string fline = ser_res.substr(0, ser_res.find("\n"));
    string http_ver =  fline.substr(0,fline.find(" "));

    string sres_code = fline.substr(fline.find(" ")+1, fline.substr(fline.find(" ")+1).find(" "));
    int res_code = stoi(sres_code);
    
    return res_code;

}

// gets the content type of the response
string get_content_type(string res)
{
    string cnt =  res.substr(res.find("Content-Type"));
    cnt = cnt.substr(0,res.substr(res.find("Content-Type")).find("\n"));
    
    cnt = cnt.substr(cnt.find(" ")+1);
    
    cnt = cnt.substr(0,cnt.find(";"));
    
    return cnt;
}

// gets random string 
string rand_str()
{
    string rs = "ab";
    for (int i = 0; i < rs.length();i++) 
    {
        rs[i] = (char) ((rand()%26) + 65);
    }
    rs = "<b>" + rs + "</b>";
    return rs;
}
