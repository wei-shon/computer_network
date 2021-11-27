#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include<time.h>

#define serverPort 48763

char *convert(char *src , char game[4]) {
    //the game is answer
    char *iter = src;
    char *result = malloc(sizeof(src));
    char *it = result;
    if (iter == NULL) return iter;
    int A=0;
    int B=0;
    int index=0;
    for(int j = 0; j < 4 ; j++){//to count what the number we guess correct or wrong or wrong position
        for(int i = 0 ; i < 4 ; i++)
        {
            if(*iter==game[i])
            {
                if(index==i)
                {
                    A++;
                }
                else
                {
                    B++;
                }
            }
        }
        index++;
        iter++;
    }
    *it+=A+'0';
    it++;
    *it+='A';
    it++;
    *it+=B+'0';
    it++;
    *it+='B';
    //printf("answer: %s\n",result);
    return result;
}


int main() 
{
    // message buffer
    char buf[1024] = {0};
    printf("%c",buf[1]);
    // 建立 socket
    int socket_fd = socket(PF_INET , SOCK_STREAM , 0);
    if (socket_fd < 0){
        printf("Fail to create a socket.");
    }
    
    // server 地址
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(serverPort)
    };
    
    // 將建立的 socket 綁定到 serverAddr 指定的 port
    if (bind(socket_fd, (const struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind socket failed!");
        close(socket_fd);
        exit(0);
    }

    // 初始化，準備接受 connect
    // backlog = 5，在 server accept 動作之前，最多允許五筆連線申請
    // 回傳 -1 代表 listen 發生錯誤
    if (listen(socket_fd, 5) == -1) {
        printf("socket %d listen failed!\n", socket_fd);
        close(socket_fd);
        exit(0);
    }

    printf("server [%s:%d] --- ready\n", 
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    while(1) {
        int reply_sockfd;
        struct sockaddr_in clientAddr;
        int client_len = sizeof(clientAddr);

        // 從 complete connection queue 中取出已連線的 socket
        // 之後用 reply_sockfd 與 client 溝通
        reply_sockfd = accept(socket_fd, (struct sockaddr *)&clientAddr, &client_len);
        printf("Accept connect request from [%s:%d]\n", 
                inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
        
        //generate the number to play
        char game[4]={-1,-1,-1,-1};
        srand(time(NULL));
        int index=0;
        while(1)
        {
            int k = rand()%10;
            int check=1;
            for(int i = 0 ; i < index ;i++)
            {
                if(k==(int)game[i]-'0')
                {
                    check=0;
                }
            }
            if(check)
            {
                game[index]=k+'0';
                index++;
            }
            if(index==4)
            {
                break;
            }
        }
        printf("the answer is : %s\n",game);

        // 不斷接收 client 資料
        while (recv(reply_sockfd, buf, sizeof(buf), 0)) {
            // 收到 exit 指令就離開迴圈
            if (strcmp(buf, "exit") == 0) {
                memset(buf, 0, sizeof(buf));
                goto exit;
            }
            
            // 顯示資料來源，原本資料，以及修改後的資料
            printf("get message from [%s:%d]: ",
                    inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            
            // print what we get from client
            printf("%s\n", buf);

            //check the input is correct or not
            char *conv;

            //If the player want to leave
            if(strcmp(buf,"leave")==0)
            {
                char temp[20]="leave";
                conv=temp;
                send(reply_sockfd, conv, sizeof(conv), 0);
                printf("game over\n");
                goto exit;                
            }


            char *temp = buf;
            int flag = 1; //to the input is 4 digits or not

            for (int i = 0 ; i < 5 ; i++)
            {
                //if the input is not fill with 4 digits 
                if(*temp == NULL && i < 4)
                {
                    flag = 0;
                }
                //if the input is oversize the 4 digits
                else if(*temp !=NULL && i==4)
                {
                    flag = 0;
                }
                temp++;
            }
            
            if(flag == 0)
            {
                conv="See Rule";
            }
            else{
                conv = convert(buf,game);
            }
            if(strcmp(conv, "4A0B") == 0)
            {
                char temp[20]="WIN";
                conv=temp;
                send(reply_sockfd, conv, sizeof(conv), 0);
                printf("you are winner!!\n");
                goto exit;
            }
            // 傳回 client 端
            // 不需要填入 client 端的位置資訊，因為已經建立 TCP 連線
            if (send(reply_sockfd, conv, sizeof(conv), 0) < 0) {
                printf("send data to %s:%d, failed!\n", 
                        inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
                memset(buf, 0, sizeof(buf));
                free(conv);
                goto exit;
            }

            // 清空 message buffer
            memset(buf, 0, sizeof(buf));
            //free(conv);
        }
        
        // 關閉 reply socket，並檢查是否關閉成功
        if (close(reply_sockfd) < 0) {
            perror("close socket failed!");
        }
    }

exit:
    // 關閉 socket，並檢查是否關閉成功
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}