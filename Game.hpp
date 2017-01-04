#ifndef GAME_HPP
#define GAME_HPP

#include "Window.hpp"
#include <map>
#include <thread>

#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

class Game {

    Window * window;
    Character* player;
    std::map<int, Character*> players;
    SDL_Event event;
    const double FRAMES_PER_SECOND = 60.0;
    bool isRunning = true;
    clock_t init;
    int width, height;

    public:
        Game() {
            width = 640;
            height = 480;
            window = new Window();
            window->createWindow("DOTS!", width, height);
            player = new Character();
            player->setImage(window->getRenderer(), "./images/block.png");
            player->setFrame(0,0,20,20);
        }
        ~Game() {
            delete window;
            window = NULL;
            delete player;
            player = NULL;
        }

        // TODO fix this version because a segmentation fault happens most likely when accessing the players map object
        void run () {

            auto th = std::thread(handleConnection, window, player, std::ref(players), std::ref(isRunning));

            while (isRunning) {
                init = clock();
                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT) {
                        isRunning = false;
                    }
                    handleGameEvents(event);
                }
            
                update();

                SDL_RenderClear(window->getRenderer());
                draw();
                SDL_RenderPresent(window->getRenderer());

                if ( ((clock() - init)/(double)CLOCKS_PER_SEC) < 1000.0/FRAMES_PER_SECOND ) {
                    SDL_Delay( (1000.0/FRAMES_PER_SECOND) - ((clock() - init)/(double)CLOCKS_PER_SEC) );
                }
            }

            th.join();
        }

        void runSynch() {
            int sockfd, portno, n;
            // 
            sockaddr_in serv_addr;
            //
            hostent *server;
            // buffer to send to server
            unsigned char write_buffer[8];
            portno = 8080;
            // establish the tcp socket. Domain, Type, Protocol; IPV4, TCP, Unspecified
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                std::cout<<"Error opening socket!\n";
                exit(1);
            }
            // returns the host by the name
            server = gethostbyname("localhost");
            if (server == NULL) {
                std::cout<<"Error, no such host!\n";
                exit(1);
            }
            // zeroing out everything
            bzero((char*) &serv_addr, sizeof(serv_addr));
            // setting the family
            serv_addr.sin_family = AF_INET;
            // copying over the server host address to the serv_addr s_addr field
            bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
            // assigning port number
            serv_addr.sin_port = htons(portno);
            // connecting our socket to the server address
            if (connect(sockfd, (sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
                std::cout<<"Error in connection!\n";
                exit(1);
            }
            // zero out the write buffer
            bzero(write_buffer, 8);
            Game::writePlayerPositionBuffer(write_buffer, player->getFrame());
            // send 8 bytes of the buffer to the socket. last param is flags
            n = send(sockfd, write_buffer, 8, 0);
            if (n < 0) {
                std::cout<<"Error writing to socket!\n";
                exit(1);
            }
            // create separate read buffer
            unsigned char read_buffer[73];
            bzero(read_buffer, 73);

            while (isRunning) {
                init = clock();
                while (SDL_PollEvent(&event) != 0) {
                    if (event.type == SDL_QUIT) {
                        send(sockfd, "exit", 4, 0);
                        isRunning = false;
                    }
                    handleGameEvents(event);
                }
                // recieve returned data from the socket. upto 72 bytes. last param is flags
                n = recv(sockfd, read_buffer, 72, 0);
                if (n < 0) {
                    std::cout<<"Error in reading buffer!\n";
                    exit(1);
                }
                if (strncmp((char*)read_buffer, "null", 4) != 0) {
                    //std::printf("read %d data %s", n, read_buffer);
                    if (strncmp((char*)read_buffer, "exit", 4) == 0) {
                        std::cout<<"disconnected..\n";
                        break;
                    }
                    Game::updatePlayers(window, players, read_buffer, n);
                }
            
                update();

                SDL_RenderClear(window->getRenderer());
                draw();
                SDL_RenderPresent(window->getRenderer());

                Game::writePlayerPositionBuffer(write_buffer, player->getFrame());
                n = send(sockfd, write_buffer, 8, 0);
                if (n < 0) {
                    std::cout<<"Error writing to socket in while loop!\n";
                    exit(1);
                }
                bzero(read_buffer, 73);

                if ( ((clock() - init)/(double)CLOCKS_PER_SEC) < 1000.0/FRAMES_PER_SECOND ) {
                    SDL_Delay( (1000.0/FRAMES_PER_SECOND) - ((clock() - init)/(double)CLOCKS_PER_SEC) );
                }
            }
            close(sockfd);
        }

        void update() {
            window->setBackgroundColor(0,0,0,255);
            conditions();
        }

        void handleGameEvents(SDL_Event& e) {
            if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    case SDLK_w:
                        player->up = true;
                        break;
                    case SDLK_s:
                        player->down = true;
                        break;
                    case SDLK_d:
                        player->right = true;
                        break;
                    case SDLK_a:
                        player->left = true;
                        break;
                }
            } else if (e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym) {
                    case SDLK_w:
                        player->up = false;
                        break;
                    case SDLK_s:
                        player->down = false;
                        break;
                    case SDLK_d:
                        player->right = false;
                        break;
                    case SDLK_a:
                        player->left = false;
                        break;
                }
            }
        }

        void conditions() {
            if (player->up) {
                player->moveYBy(-5);
                if (player->getFrame()->y < 0) {
                    player->updatePosition(player->getFrame()->x, player->getFrame()->y + (height));
                }
            } else if (player->down) {
                player->moveYBy(5);
                if (player->getFrame()->y > height) {
                    player->updatePosition(player->getFrame()->x, player->getFrame()->y - (height));
                }
            } else if (player->left) {
                player->moveXBy(-5);
                if (player->getFrame()->x < 0) {
                    player->updatePosition(player->getFrame()->x + (width), player->getFrame()->y);
                }
            } else if (player->right) {
                player->moveXBy(5);
                if (player->getFrame()->x > width) {
                    player->updatePosition(player->getFrame()->x - (width), player->getFrame()->y);
                }
            }
        }

        void draw() {
            window->drawObjectToScreen(player);
            for (auto it = players.begin(); it != players.end(); ++it) {
                window->drawObjectToScreen(it->second);
            }
        }

        static void handleConnection(Window* window, Character* player, std::map<int, Character*>& players, bool& isRunning) {
            // sockfd is the file descriptor to a socket(because sockets are treated as files)
            // portno is port number.
            // n will be set to the return value of write and read and is the number of bytes read or written.
            int sockfd, portno, n;
            // 
            sockaddr_in serv_addr;
            //
            hostent *server;
            // buffer to send to server
            unsigned char write_buffer[8];
            portno = 8080;
            // establish the tcp socket. Domain, Type, Protocol; IPV4, TCP, Unspecified
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd < 0) {
                std::cout<<"Error opening socket!\n";
                exit(1);
            }
            // returns the host by the name
            server = gethostbyname("localhost");
            if (server == NULL) {
                std::cout<<"Error, no such host!\n";
                exit(1);
            }
            // zeroing out everything
            bzero((char*) &serv_addr, sizeof(serv_addr));
            // setting the family
            serv_addr.sin_family = AF_INET;
            // copying over the server host address to the serv_addr s_addr field
            bcopy((char*) server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length);
            // assigning port number
            serv_addr.sin_port = htons(portno);
            // connecting our socket to the server address
            if (connect(sockfd, (sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
                std::cout<<"Error in connection!\n";
                exit(1);
            }
            // zero out the write buffer
            bzero(write_buffer, 8);
            writePlayerPositionBuffer(write_buffer, player->getFrame());
            // send 8 bytes of the buffer to the socket. last param is flags
            n = send(sockfd, write_buffer, 8, 0);
            if (n < 0) {
                std::cout<<"Error writing to socket!\n";
                exit(1);
            }
            // create separate read buffer
            unsigned char read_buffer[73];
            bzero(read_buffer, 73);
            while (true) {
                // recieve returned data from the socket. upto 72 bytes. last param is flags
                n = recv(sockfd, read_buffer, 72, 0);
                if (n < 0) {
                    std::cout<<"Error in reading buffer!\n";
                    exit(1);
                }
                if (strncmp((char*)read_buffer, "null", 4) != 0) {
                    //std::printf("read %d data %s", n, read_buffer);
                    if (strncmp((char*)read_buffer, "exit", 4) == 0) {
                        std::cout<<"disconnected..\n";
                        break;
                    }
                    updatePlayers(window, players, read_buffer, n);
                }
                writePlayerPositionBuffer(write_buffer, player->getFrame());
                n = send(sockfd, write_buffer, 8, 0);
                if (n < 0) {
                    std::cout<<"Error writing to socket in while loop!\n";
                    exit(1);
                }
                bzero(read_buffer, 73);
            }
            // close out socket
            close(sockfd);
        }




        static void updatePlayers(Window* window, std::map<int, Character*>& tmp_map, unsigned char buf[], int n) {
            int tmp_id, tmp_x, tmp_y;
            for (int i = 0; i < n; i += 12) {
                tmp_id = ((int)buf[i]) + (((int)buf[i+1])<<8) + (((int)buf[i+2])<<16) + (((int)buf[i+3])<<24);
                tmp_x = ((int)buf[i+4]) + (((int)buf[i+5])<<8) + (((int)buf[i+6])<<16) + (((int)buf[i+7])<<24);
                tmp_y = ((int)buf[i+8]) + (((int)buf[i+9])<<8) + (((int)buf[i+10])<<16) + (((int)buf[i+11])<<24);
                auto it = tmp_map.find(tmp_id);
                if (it == tmp_map.end()) {
                    Character* tmp_chr = new Character();
                    tmp_chr->setFrame(tmp_x, tmp_y, 20, 20);
                    tmp_chr->setImage(window->getRenderer(), "./images/block.png");
                    tmp_map[tmp_id] = tmp_chr;
                } else {
                    tmp_map[tmp_id]->updatePosition(tmp_x, tmp_y);
                }
            } 
        }


        static void writePlayerPositionBuffer(unsigned char arr[], SDL_Rect* rect) {
            int shift = 0;
            for (int i = 0; i<4; ++i) {
                arr[i]   = uint8_t(rect->x>>shift);
                arr[i+4] = uint8_t(rect->y>>shift);
                shift += 8;
            }
        }
};

#endif
