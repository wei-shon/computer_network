#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#define serverPort 48763
#define serverIP "127.0.0.1"


int main() 
{
    // message buffer
    char buf[1024] = {0};
    char recvbuf[1024] = {0};
    int counter = 0;

    // 建立 socket
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    // server 地址
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(serverIP),
        .sin_port = htons(serverPort)
    };
    int len = sizeof(serverAddr);

    // 試圖連結 server，發起 tcp 連線
    // 回傳 -1 代表 server 可能還沒有開始 listen
    if (connect(socket_fd, (struct sockaddr *)&serverAddr, len) == -1) {
        printf("Connect server failed!\n");
        close(socket_fd);
        exit(0);
    }

    printf("Connect server [%s:%d] success\n\n",
            inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

    printf("welcome to AB game!!!\n\n");
    printf("you need to find true number\n\n");
    printf("you should enter the 4 number\n\n");
    printf("e.g. 1234 or 6541\n\n");
    printf("Do not enter the same number\n\n");
    printf("e.g. 1122 or 1123 or 1223 or 1233......\n\n");
    printf("If you get the true number then you win!!!!\n\n");
    printf("If you want to leave the game\n\n");
    printf("Input 'leave' and then you can leave the game\n\n");

    while (1) {
        // 輸入資料到 buffer
        printf("Please input your guess number: ");
        scanf("%s", buf);
        printf("\n");
        counter++;
        // 傳送到 server 端
        if (send(socket_fd, buf, sizeof(buf), 0) < 0) {
            printf("send data to %s:%d, failed!\n", 
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            memset(buf, 0, sizeof(buf));
            break;
        }
        
        // 接收到 exit 指令就退出迴圈
        if (strcmp(buf, "exit") == 0)
            break;
        
        // 清空 message buffer
        memset(buf, 0, sizeof(buf));

        // 等待 server 回傳轉成大寫的資料
        if (recv(socket_fd, recvbuf, sizeof(recvbuf), 0) < 0) {
            printf("recv data from %s:%d, failed!\n", 
                    inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
            break;
        }

        //if get WIN it's means successfully guess the answer
        if (strcmp(recvbuf, "WIN") == 0)
        {
            printf("YOU WIN!!!!\n");
            printf("YOU USE THE %d times to win\n",counter);
            break;
        }

        //See Rule
        if (strcmp(recvbuf, "leave") == 0)
        {
            printf("OK!!!!\n");
            printf("The game is over\n");
            break;
        }
        //
        if (strcmp(recvbuf, "See Rule") == 0)
        {
            printf("Please watch the rules again!!!!\n");
            printf("Do not use the same digits or that less or more than 4 digits!!!!\n\n");
            continue;
        }
        // 顯示 server 地址，以及收到的資料
        printf("your guess number's status is : %s\n\n", recvbuf);
        memset(recvbuf, 0, sizeof(recvbuf));
    }

    // 關閉 socket，並檢查是否關閉成功
    if (close(socket_fd) < 0) {
        perror("close socket failed!");
    }
    return 0;
}