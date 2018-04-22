/*******************************************************************
*   支持任意多的空格插入。能区分 | 和 || 但是对 || 无响应。能区分< > >> 不支持除空格 | || < > >>之外的分隔符。指令开头可以加空格。
*   支持管道，理论上支持最多128个管道，可以随便加空格，支持ls|wc|wc这种没有空格的形式
*   支持设置环境变量
*   支持文件重定向。 可以使用< > >>三种符号，支持>或>>到多个文件中，支持随意加空格，支持><>>和|管道连用。可以识别wc<a|wc>b>>c这种没有空格的格式
*******************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <stdlib.h>

//#define splitDebug

typedef enum bool{
    false = 0, true = 1
}bool;

typedef struct myQueue{
    int data[256];
    int rear;
}myQueue;

void QueueTraverseClose(myQueue *Q){     //遍历队列关闭所有文件
    int i;
    for(i = 0; i < Q -> rear; i ++){
        close(Q -> data[i]);
    }
}

void ClearQueue(myQueue *Q){
    int i;
    for(i = 0; i < Q -> rear; i ++){
        Q -> data[i] = 0;
    }
    Q -> rear = 0;
}

void EnQueue(myQueue *Q, int d){
    Q -> data[Q -> rear ++] = d;
}

bool QueueEmpty(myQueue Q){
    if(Q.rear == 0)return true;
    else return false;
}

void closeAll(int pipenum, int pipefd[][2]){    //关掉0-pipenum的二维数组管道口
    int i;
    for(i = 0; i <= pipenum; i ++){
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
}

//我本来以为 管道的意思是 上一个的输出 转化成字符串作为 下一个指令的参数， 所以写了这个函数，而实际并没用用上。
char **ptrmerge(char **a, char **b){      //从a到null，然后从b到null，合并成一个指针数组并返回，以null结尾
    static char *ptra[300], **t;
    t = ptra;
    while(*t ++ = *a ++);
    t --;   //去掉null
    while(*t ++ = *b ++);
    return ptra;
}

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    int saveStdin = dup(0);
    int saveStdout = dup(1);
    while (1) {
        dup2(saveStdin, 0);
        dup2(saveStdout, 1);
        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        if(i > 252){
            printf("instruction too long!\n");          //指令太长
            continue;
        }
        cmd[i] = '\0';
        /* 拆解命令行 */             //可以插入任意空格 。现在不能按下上下左右箭头，不能ctrl+L等，能基本分离||和|，但是如||a这样的从开头写的错误指令无法分离
        for(args[0] = cmd; *args[0] == ' '; args[0] ++); //找到第一个不是空格的位置
        for (i = 0; *args[i]; i++){
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++){
                if (*args[i+1] == ' ') {
                    *args[i+1] = '\0';
                    loop:for(args[i+1] ++; *args[i+1] == ' '; args[i+1] ++);   //找到不是空格的第一个字符位置
                    if(*args[i+1] == '|' || *args[i+1] == '<' || *args[i+1] == '>'){      // | 后没有空格的情况 排除 ||
                        if(*(args[i+1] + 1) != '|' && *(args[i+1] + 1) != '>'){    //不是 ||
                            args[i+2] = args[i+1];
                            switch(*args[i+1]){
                                case '|': args[++i] = "|"; break;
                                case '>': args[++i] = ">"; break;
                                case '<': args[++i] = "<"; break;
                            }
                        }
                        else {
                            if(*args[i+1] == '|' && *(args[i+1] + 1) == '|'){    // 是||
                                args[i+2] = args[i+1] + 1;
                                args[++i] = "||";
                            }
                            else if(*args[i+1] == '>' && *(args[i+1] + 1) == '>'){// 是>>
                                args[i+2] = args[i+1] + 1;
                                args[++i] = ">>";
                            }
                        }
                        goto loop;
                    }
                    break;
                }
                else if(*args[i+1] == '|' || *args[i+1] == '>' || *args[i+1] == '<'){
                    char sav = *args[i+1];
                    *args[i+1] = '\0';
                    if(*(args[i+1] + 1) != '|' && *(args[i+1] + 1) != '>'){
                        args[i+2] = args[i+1];
                        switch(sav){
                            case '|': args[++i] = "|"; break;
                            case '>': args[++i] = ">"; break;
                            case '<': args[++i] = "<"; break;
                        }
                    }
                    else {
                        if(sav == '|' && *(args[i+1] + 1) == '|'){    // 是||
                            args[i+2] = args[i+1] + 1;
                            args[++i] = "||";
                        }
                        else if(sav == '>' && *(args[i+1] + 1) == '>'){// 是>>
                            args[i+2] = args[i+1] + 1;
                            args[++i] = ">>";
                        }
                    }
                    for(args[i+1] ++; *args[i+1] == ' '; args[i+1] ++);   //找到不是空格的第一个字符位置
                    break;
                }
            }
        }
        args[i] = NULL;

#ifdef splitDebug
        for(i = 0; args[i]; i ++){
            puts(args[i]);
        }
        continue;
#endif // splitDebug

        int pos = 0, pipenum = 0;
        char **newcmd[128];
        newcmd[0] = args;
        myQueue Qa, Qw; //追加队列、重写队列
        ClearQueue(&Qa);
        ClearQueue(&Qw);

        for(i = 0; args[i]; i ++){      //统计管道数 ls -l | wc NULL
            if(*args[i] == '|'){
                pipenum ++;
                newcmd[pipenum] = args + i + 1;
                args[i] = NULL;     //把指向管道符 | 转化为 NULL
            }
            else if(*args[i] == '<'){
                int fd = open(args[i + 1], O_RDONLY, 0666);
                if(fd < 0){
                    printf("file not found!\n");
                    continue;
                }
                dup2(fd, 0);    //从文件中读
                close(fd);
                int j;
                for(j = i; ; j ++){     //在指针数组里删除掉这个文件名
                    if((args[j] = args[j + 2]) == NULL) break;
                }
                i --;       //重读这一位
            }
            else if(*args[i] == '>'){
                if(*(args[i] + 1) == '>'){  //>>
                    int fd = open(args[i + 1], O_RDWR | O_APPEND | O_CREAT, 0666);
                    if(fd < 0){
                        printf("cannot open file!\n");
                        continue;
                    }
                    EnQueue(&Qa, fd); //先入队Qa
                }
                else {          //>
                    int fd = open(args[i + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
                    if(fd < 0){
                        printf("cannot open file!\n");
                        continue;
                    }
                    EnQueue(&Qw, fd);//先入队Qw
                }
                int kk;
                for(kk = i; ; kk ++){     //在指针数组里删除掉这个文件名
                    if((args[kk] = args[kk + 2]) == NULL) break;
                }
                i --;       //重读这一位
            }
        }
        int temppipe[2];
        if(!QueueEmpty(Qa) || !QueueEmpty(Qw)){ //需要写文件
            pipe(temppipe);
            dup2(temppipe[1], 1);   //先写到管道中    如果是>或 >> 有时可能要写入多个文件，所以先写入管道，最后再把管道的内容读到多个文件中。
            close(temppipe[1]);
        }
        if(pipenum == 0)goto next;  //没有管道
        int pipefd[128][2];
        //memset(pipefd, 0, sizeof(int) * 128 * 2);
        for(i = 0; i <= pipenum; i ++){     //pipenum + 1次
            if(i < pipenum)pipe(pipefd[i]); //pipenum次
            pid_t pid = fork();//pipenum + 1次
            if (pid < 0){
                printf("fork failed!\n");       //fork失败
                return(255);
            }
            else if(pid == 0)break; //子进程跳出循环
            //父进程继续这个循环
        }
                /* 子进程 */
                if(i == 0){     //第一个子进程
                    dup2(pipefd[0][1], 1);  //写到管道中
                    closeAll(i, pipefd);
                    execvp(*newcmd[i], newcmd[i]);
                    /* execvp失败 */
                    printf("no such instruction!\n");        //未定义指令
                    return 255;
                }
                else if(i == pipenum){  //最后一个进程
                    dup2(pipefd[pipenum - 1][0], 0);  //从管道中读
                    closeAll(i - 1, pipefd);
                    execvp(*newcmd[i], newcmd[i]);
                    /* execvp失败 */
                    printf("no such instruction!\n");        //未定义指令
                    return 255;
                }
                else if(i < pipenum){   //其他进程
                    dup2(pipefd[i - 1][0], 0);
                    dup2(pipefd[i][1], 1);
                    closeAll(i, pipefd);
                    execvp(*newcmd[i], newcmd[i]);
                    /* execvp失败 */
                    printf("no such instruction!\n");        //未定义指令
                    return 255;
                }
                else {              //父进程
                    closeAll(pipenum - 1, pipefd);
                    for(i = 0; i <= pipenum; i ++)
                        wait(NULL);
//                    if(i == 0)close(pipefd[i][1]);
//                    else if(i == pipenum)close(pipefd[i - 1][0]);
////                    else {
////                        close(pipefd[i - 1][0]);
////                        close(pipefd[i][1]);
////                    }
                }
        goto check;
        continue;

        /* 没有输入命令 */
     next:   if (!args[pos])
            continue;

        /* 内建命令 */
        if (strcmp(args[pos], "cd") == 0) {
            if (args[1])
                if(chdir(args[1]) < 0) printf("change directory failed!\n");
            continue;
        }
        if (strcmp(args[pos], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            goto check;
            continue;
        }
        if (strcmp(args[pos], "exit") == 0)
            return 0;

        if (strcmp(args[pos], "export") == 0){
            char *p = args[1];
            while(*p && *p != '=')p ++;
            *p ++ = '\0';
            if(setenv(args[1], p, 1) < 0)printf("setenv error\n");
            continue;
        }

        /* 外部命令 */
        pid_t pid = fork();
        if (pid < 0){
            printf("fork failed!\n");       //fork失败
            continue;
        }
        else if (pid == 0) {
            /* 子进程 */
            execvp(args[pos], args);    //ptrmerge(args[pos], pipeargs[0])
            /* execvp失败 */
            printf("no such instruction!\n");        //未定义指令
            return 255;
        }
        /* 父进程 */
        wait(NULL);     //等待子进程结束
        check:if(!QueueEmpty(Qa) || !QueueEmpty(Qw)){ //需要写文件
            char buf[5000];
            memset(buf, 0, 5000);
            int bn = read(temppipe[0], buf, sizeof(buf));
            close(temppipe[0]);
            int i;
            for(i = 0; i < Qw.rear; i ++){
                write(Qw.data[i], buf, bn);
                close(Qw.data[i]);
            }
            for(i = 0; i < Qa.rear; i ++){
                write(Qa.data[i], buf, bn);
                close(Qa.data[i]);
            }

        }
    }
}
