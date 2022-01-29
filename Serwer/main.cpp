#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <arpa/inet.h>
#include <cstdlib>
#include <unistd.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <algorithm>
#include <list>
#include "dist/json/json.h"
#include "jsoncpp.cpp"
#include "ProtocolHead/HeadData.h"
#include "Service/DataProcesser.h"
#include "config/server_config.h"
using namespace std;
list <string> codeGenerator;
list <string> activeGames;
multimap<string,int> playersCodes;
map<string,string> activeCreators;
multimap<int,Json::Value> questions;
list <string> activeUsers;
map<string,map<string,int>> ranking;
map<int,string> activeUsersfd;
int main() {
    for(int i=100; i<110; i++)
    {
        codeGenerator.push_back(to_string(i));
    }
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{}, clientAddr{};
    int opt = 1;
    if (-1 == setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        cout << "setsockopt fail" << endl;
        exit(-1);
    }
    int epfd = epoll_create(MAX_CONNECTIONS);
    epoll_event ev{}, events[MAX_CONNECTIONS];
    ev.data.fd = lfd;
    ev.events = EPOLLIN;
    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev)) {
        cout << "epoll_ctl fail" << endl;
        exit(-1);
    }
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, HOST, &serverAddr.sin_addr);
    if (-1 == bind(lfd, (sockaddr *) &serverAddr, sizeof(serverAddr))) {
        cout << "Bind fail" << endl;
        exit(-1);
    }
    if (-1 == listen(lfd, MAX_CONNECTIONS)) {
        cout << "Listen fail" << endl;
        exit(-1);
    }
    cout << "Listening" << endl;
    char ipAddress[BUFSIZ];
    while (true) {
        int nready = epoll_wait(epfd, events, MAX_CONNECTIONS, -1);
        if (nready < 0) {
            cout << "epoll_wait error" << endl;
            exit(-1);
        }
        cout <<"Incoming connections: "<<nready <<endl;
        for (int i = 0; i < nready; i++) {
            int fd = events[i].data.fd;
            if (fd == lfd) {
                socklen_t len = sizeof(clientAddr);
                int cfd = accept(lfd, (sockaddr *) &clientAddr, &len);
                ev.data.fd = cfd;
                ev.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
                inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(clientAddr));
                struct timeval timeout = {1, 0};
                setsockopt(cfd, SOL_SOCKET, TCP_NODELAY, (char *) &timeout, sizeof(struct timeval));
            } else if (events[i].events & EPOLLIN) {
                HeadData hd(fd);
                unsigned int protocolId = hd.getProtocolId();
                unsigned int account = hd.getAccount();
                unsigned int dataType = hd.getDataType();
                unsigned int dataLength = hd.getDataLength();
                DataProcesser dp;
                switch (protocolId) {
                    case SENDQUESTION:
                    {
                        cout<<"QUESTION SET RECEIVED"<<endl;
                        string quesMsg = dp.readTextContent(fd, dataLength);
                        cout<<quesMsg<<endl;
                        Json::Reader jsonReader;
                        Json::Value msg;
                        jsonReader.parse(quesMsg, msg);
                        string id = msg["id"].asString();
                        string code = msg["code"].asString();
                        string question = msg["ques"].asString();
                        string answer1 = msg["anso"].asString();
                        string answer2 = msg["anst"].asString();
                        string answer3 = msg["ansth"].asString();
                        string answer4 = msg["ansf"].asString();
                        string correct = msg["cor"].asString();
                        string time = msg["time"].asString();
                        string user = msg["creator"].asString();
                        Json::Value questionResult;
                        if(code==activeCreators[user])
                        {
                            questions.insert(pair<int,Json::Value>(fd,msg));
                            questionResult["type"] = ACCEPTQUESTION;
                            questionResult["text"] = "123";
                        }
                        else{
                            cout<<"FAIL"<<endl;
                        }
                        /*for (auto itr = questions.begin();itr!=questions.end(); ++itr)
                            {
                                cout<< itr->first << '\t' << itr->second<<'\n';
                            }*/
                        string questionResultStr = questionResult.toStyledString();
                        dp.writeMsg(fd, 0, questionResultStr);

                    }
                        break;
                    case REQUESTCODE:
                    {
                        string codeMsg = dp.readTextContent(fd, dataLength);
                        Json::Reader jsonReader;
                        Json::Value msg;
                        jsonReader.parse(codeMsg, msg);
                        string type = msg["type"].asString();
                        Json::Value codeResult;
                        if(type=="request")
                        {
                            codeResult["type"] = REQUESTCODE_SEND;
                            codeResult["code"] = stoi(codeGenerator.front());
                            activeGames.push_back(codeGenerator.front());
                            for (auto itr = activeUsersfd.begin();itr!=activeUsersfd.end();itr++)
                        {
                            if(itr->first==fd)
                            {
                                activeCreators.insert({itr->second,codeGenerator.front()});
                            }
                            //cout<< itr->first<<'\t'<<itr->second<<'\n';
                        }
                            
                            codeGenerator.pop_front();
                            /*for(auto v: activeGames)
                            {
                                cout<<v<<endl;
                            }
                            cout<<"GENERATOR"<<endl;
                            for(auto v: codeGenerator)
                            {
                                cout<<v<<endl;
                            }
                            cout<<"Kod,fd"<<endl;
                            for (auto itr = activeCreators.begin();itr!=activeCreators.end(); ++itr)
                            {
                                cout<< itr->first << '\t' << itr->second<<'\n';
                            }*/
                        }
                        else{
                            cout<<"FAIL"<<endl;
                        }
                        string codeResultStr = codeResult.toStyledString();
                        //cout<<codeResultStr<<endl;
                        dp.writeMsg(fd, 0, codeResultStr);
                        //cout<<"CODE SENT"<<endl;
                    }
                        break;
                    case STARTGAME:
                    {
                        /*Json::Value codeResult;
                        codeResult["type"] = NOTICE;
                        string codeResultStr = codeResult.toStyledString();
                        dp.writeMsg(7,0,codeResultStr);*/
                        vector <int> fdsPlayers;
                        string codeMsg = dp.readTextContent(fd, dataLength);
                        Json::Value codeResult;
                        Json::Reader jsonReader;
                        Json::Value msg;
                        jsonReader.parse(codeMsg, msg);
                        string code = msg["code"].asString();
                        //string creator=
                       // string a=activeCreators[fd];
                        //string codersl;
                       // cout<<a<<endl;
                       int playersingame=0;
                       for(auto itr = playersCodes.find(code);itr!=playersCodes.end();itr++)
                       {
                           playersingame++;
                       }
                       if(playersingame>0)
                       {
                           for(const auto& elem : playersCodes)
                        {
                            if(elem.first==code)
                            {
                                fdsPlayers.push_back(elem.second);
                            }
                        }
                        int i=0;
                        for(const auto& elem : questions)
                        {   
                            if(elem.first==fd)
                            {
                                i++;
                                codeResult[to_string(i)]=elem.second;
                            }
                        }
                        /*for (auto itr = questions.begin();itr!=questions.end(); ++itr)
                            {
                                cout<< itr->first << '\t' << itr->second<<'\n';
                            }
                        for(int i: fdsPlayers)
                        {
                            cout<<i<<endl;
                        }*/
                        codeResult["type"] = NOTICE;
                        codeResult["number"]= i;
                       /* for (size_t i=0; i<fdsPlayers.size();i++)
                        {
                            cout<<fdsPlayers[i]<<endl;
                        }*/
                        string codeResultStr = codeResult.toStyledString();
                        //codeResultStr.erase(std::remove(codeResultStr.begin(), codeResultStr.end(), '\\'), codeResultStr.end());
                        cout<<"PO USUNIECIU: "<<endl<<codeResultStr<<endl;
                        dp.writeTextToAllUser(fdsPlayers,fd,codeResultStr,NOTICE);
                       }
                       else
                       {
                           codeResult["type"]=NOTENOUGHPLAYERS;
                       }
                        string codeResultStr = codeResult.toStyledString();
                        dp.writeMsg(fd, 0, codeResultStr);
                    }
                        break;
                    case ANSWER:
                    {
                        map<string,map<string,int>>::iterator itr;
                        map<string,int>::iterator ptr;
                        string codeMsg = dp.readTextContent(fd, dataLength);
                        Json::Value codeResult;
                        Json::Reader jsonReader;
                        Json::Value msg;
                        jsonReader.parse(codeMsg, msg);
                        string info = msg["info"].asString();
                        string code = msg["code"].asString();
                        string name = msg["name"].asString();
                        cout<<msg<<endl;
                        if(info=="correct")
                        {
                            //cout<<"poprawna odp"<<endl;
                            //ranking.insert[{code,{fd,}}]
                            for(itr=ranking.begin();itr!=ranking.end();itr++)
                            {
                                for(ptr=itr->second.begin();ptr!=itr->second.end();ptr++)
                                {
                                    if(itr->first==code)
                                    {
                                        //cout<<"KOD GIT"<<endl;
                                        if(ptr->first==name)
                                        {
                                            //cout<<"username git"<<endl;
                                            ptr->second=ptr->second+100;
                                        }
                                    }
                                }
                            }
                        }
                        /*for(itr=ranking.begin();itr!=ranking.end();itr++)
                            {
                                for(ptr=itr->second.begin();ptr!=itr->second.end();ptr++)
                                {
                                    cout<<"Gamecode: "<<itr->first<<" Player:"<<ptr->first<<" Points:"<<ptr->second<<endl;
                                }
                            }*/
                    }
                        break;
                    case CODECHECKPLAYER:
                    {
                        string codeMsg = dp.readTextContent(fd, dataLength);
                        Json::Reader jsonReader;
                        Json::Value msg;
                        jsonReader.parse(codeMsg, msg);
                        string code = msg["code"].asString();
                        Json::Value codeResult;
                        bool found = (find(activeGames.begin(),activeGames.end(),code)!=activeGames.end());
                        if(found)
                        {
                            codeResult["type"] = CODECHECKPLAYER_ACCEPT;
                            codeResult["code"] = code;
                            playersCodes.insert(pair<string,int>(code,fd));
                            //ranking.insert(code,(fd,0));
                            for (auto itr = activeUsersfd.begin();itr!=activeUsersfd.end();itr++)
                        {
                            if(itr->first==fd)
                            {
                                ranking[code].insert(make_pair(itr->second,0));
                            }
                            //cout<< itr->first<<'\t'<<itr->second<<'\n';
                        }
                        }
                        else
                        {
                            cout<<"FAIL"<<endl;
                        }
                        string codeResultStr = codeResult.toStyledString();
                        dp.writeMsg(fd, 0, codeResultStr);
                        //cout<<"CODE SENT"<<endl;
                    }
                        break;
                    case LOGIN: {
                        string loginMsg = dp.readTextContent(fd, dataLength);
                        Json::Reader jsonReader;
                        Json::Value msg;
                        jsonReader.parse(loginMsg, msg);
                        string account = msg["account"].asString();
                        bool found = (find(activeUsers.begin(),activeUsers.end(),account)!=activeUsers.end());
                        Json::Value loginResult;
                        //Succesful login
                        if (found) 
                        {
                            loginResult["status"] = LOGIN_EXIST;
                            cout<<"Exists";
                            } 
                        else {
                                loginResult["status"] = LOGIN_SUCCESS;
                                loginResult["username"] = 1/*user.second*/;
                                activeUsers.push_back(account);
                                activeUsersfd.insert({fd,account});
                            }
                        /*for (auto itr = activeUsersfd.begin();itr!=activeUsersfd.end();itr++)
                        {
                            cout<< itr->first<<'\t'<<itr->second<<'\n';
                        }*/
                        //Login failed
                        /*else {
                            loginResult["status"] = LOGIN_FAIL;
                            cout<<"Fail";
                        }*/
                        string loginResultStr = loginResult.toStyledString();
                        dp.writeMsg(fd, 0, loginResultStr);
                        //cout<<"LOGIN MESSAGE SENT"<<endl;
                    }
                        break;
                    case ADMIN: {
                        map<string,map<string,int>>::iterator itr;
                        map<string,int>::iterator ptr;
                        string registerMsg = dp.readTextContent(fd, dataLength);
                        Json::Reader jsonReader;
                        Json::Value adminResult;
                        Json::Value msg;
                        jsonReader.parse(registerMsg, msg);
                        string games;
                        int i=0;
                        for (auto itr = activeCreators.begin();itr!=activeCreators.end(); ++itr)
                            {   
                                i++;
                                adminResult[to_string(i)]="Creator name: "+itr->first+" Game id:"+itr->second;
                                cout<< itr->first << '\t' << itr->second<<'\n';
                            }
                        cout<<adminResult;
                        int j=0;
                        for(itr=ranking.begin();itr!=ranking.end();itr++)
                            {
                                for(ptr=itr->second.begin();ptr!=itr->second.end();ptr++)
                                {
                                    j++;
                                    string index = "Rank"+to_string(j);
                                    adminResult[index]="Gamecode: "+itr->first+" Player:"+ptr->first+" Points:"+to_string(ptr->second);
                                    //cout<<"Gamecode: "<<itr->first<<" Player:"<<ptr->first<<" Points:"<<ptr->second<<endl;
                                }
                            }
                        adminResult["gamesn"]=to_string(i);
                        adminResult["playersn"]=to_string(j);
                        adminResult["type"]=ADMIN_GAMES;
                        cout<<adminResult; 
                        dp.writeMsg(fd,0,adminResult.toStyledString(),ADMIN_GAMES);
                       // dp.writeMsg(fd,0,adminResult.toStyledString(),ADMIN_GAMES);
                        /*if (us.isRegistered(account) || !us.registerUser(account, username, password)) {
                            registerResult["status"] = REGISTER_FAIL;
                        } else {
                            registerResult["status"] = REGISTER_SUCCESS;
                        }
                        dp.writeMsg(fd, 0, registerResult.toStyledString(), REGISTER);*/
                    }
                        break;
                    case CREATOR_RANK: {
                        map<string,map<string,int>>::iterator itr;
                        map<string,int>::iterator ptr;
                        string registerMsg = dp.readTextContent(fd, dataLength);
                        Json::Reader jsonReader;
                        Json::Value adminResult;
                        Json::Value msg;
                        jsonReader.parse(registerMsg, msg);
                        string code = msg["code"].asString();
                        string games;
                        int j=0;
                        for(itr=ranking.begin();itr!=ranking.end();itr++)
                            {
                                for(ptr=itr->second.begin();ptr!=itr->second.end();ptr++)
                                {
                                    if(itr->first==code)
                                    {
                                        j++;
                                        string index = "Rank"+to_string(j);
                                        adminResult[index]="Gamecode: "+itr->first+" Player:"+ptr->first+" Points:"+to_string(ptr->second);
                                        //cout<<"Gamecode: "<<itr->first<<" Player:"<<ptr->first<<" Points:"<<ptr->second<<endl;
                                    }
                                    
                                }
                            }
                        adminResult["playersn"]=to_string(j);
                        adminResult["type"]=CREATOR_RANK_SEND;
                        cout<<adminResult; 
                        dp.writeMsg(fd,0,adminResult.toStyledString(),CREATOR_RANK_SEND);
                    }
                        break;
                    case CLOSE: {
                        sleep(1);
                        ev.data.fd = fd;
                        ev.events = EPOLLIN;
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
                        close(fd);
                        sleep(10000000);
                    }
                        break;
                    default:
                        break;
                }

            }
        }
    }

    close(lfd);
    return 0;
}
