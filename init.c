/*******************************************************************
*   ֧�������Ŀո���롣������ | �� || ���Ƕ� || ����Ӧ��������< > >> ��֧�ֳ��ո� | || < > >>֮��ķָ�����ָ�ͷ���Լӿո�
*   ֧�ֹܵ���������֧�����128���ܵ����������ӿո�֧��ls|wc|wc����û�пո����ʽ
*   ֧�����û�������
*   ֧���ļ��ض��� ����ʹ��< > >>���ַ��ţ�֧��>��>>������ļ��У�֧������ӿո�֧��><>>��|�ܵ����á�����ʶ��wc<a|wc>b>>c����û�пո�ĸ�ʽ
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

void QueueTraverseClose(myQueue *Q){     //�������йر������ļ�
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

void closeAll(int pipenum, int pipefd[][2]){    //�ص�0-pipenum�Ķ�ά����ܵ���
    int i;
    for(i = 0; i <= pipenum; i ++){
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
}

//�ұ�����Ϊ �ܵ�����˼�� ��һ������� ת�����ַ�����Ϊ ��һ��ָ��Ĳ����� ����д�������������ʵ�ʲ�û�����ϡ�
char **ptrmerge(char **a, char **b){      //��a��null��Ȼ���b��null���ϲ���һ��ָ�����鲢���أ���null��β
    static char *ptra[300], **t;
    t = ptra;
    while(*t ++ = *a ++);
    t --;   //ȥ��null
    while(*t ++ = *b ++);
    return ptra;
}

int main() {
    /* ����������� */
    char cmd[256];
    /* �����в��ɵĸ����֣��Կ�ָ���β */
    char *args[128];
    int saveStdin = dup(0);
    int saveStdout = dup(1);
    while (1) {
        dup2(saveStdin, 0);
        dup2(saveStdout, 1);
        /* ��ʾ�� */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* �����β�Ļ��з� */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        if(i > 252){
            printf("instruction too long!\n");          //ָ��̫��
            continue;
        }
        cmd[i] = '\0';
        /* ��������� */             //���Բ�������ո� �����ڲ��ܰ����������Ҽ�ͷ������ctrl+L�ȣ��ܻ�������||��|��������||a�����Ĵӿ�ͷд�Ĵ���ָ���޷�����
        for(args[0] = cmd; *args[0] == ' '; args[0] ++); //�ҵ���һ�����ǿո��λ��
        for (i = 0; *args[i]; i++){
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++){
                if (*args[i+1] == ' ') {
                    *args[i+1] = '\0';
                    loop:for(args[i+1] ++; *args[i+1] == ' '; args[i+1] ++);   //�ҵ����ǿո�ĵ�һ���ַ�λ��
                    if(*args[i+1] == '|' || *args[i+1] == '<' || *args[i+1] == '>'){      // | ��û�пո����� �ų� ||
                        if(*(args[i+1] + 1) != '|' && *(args[i+1] + 1) != '>'){    //���� ||
                            args[i+2] = args[i+1];
                            switch(*args[i+1]){
                                case '|': args[++i] = "|"; break;
                                case '>': args[++i] = ">"; break;
                                case '<': args[++i] = "<"; break;
                            }
                        }
                        else {
                            if(*args[i+1] == '|' && *(args[i+1] + 1) == '|'){    // ��||
                                args[i+2] = args[i+1] + 1;
                                args[++i] = "||";
                            }
                            else if(*args[i+1] == '>' && *(args[i+1] + 1) == '>'){// ��>>
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
                        if(sav == '|' && *(args[i+1] + 1) == '|'){    // ��||
                            args[i+2] = args[i+1] + 1;
                            args[++i] = "||";
                        }
                        else if(sav == '>' && *(args[i+1] + 1) == '>'){// ��>>
                            args[i+2] = args[i+1] + 1;
                            args[++i] = ">>";
                        }
                    }
                    for(args[i+1] ++; *args[i+1] == ' '; args[i+1] ++);   //�ҵ����ǿո�ĵ�һ���ַ�λ��
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
        myQueue Qa, Qw; //׷�Ӷ��С���д����
        ClearQueue(&Qa);
        ClearQueue(&Qw);

        for(i = 0; args[i]; i ++){      //ͳ�ƹܵ��� ls -l | wc NULL
            if(*args[i] == '|'){
                pipenum ++;
                newcmd[pipenum] = args + i + 1;
                args[i] = NULL;     //��ָ��ܵ��� | ת��Ϊ NULL
            }
            else if(*args[i] == '<'){
                int fd = open(args[i + 1], O_RDONLY, 0666);
                if(fd < 0){
                    printf("file not found!\n");
                    continue;
                }
                dup2(fd, 0);    //���ļ��ж�
                close(fd);
                int j;
                for(j = i; ; j ++){     //��ָ��������ɾ��������ļ���
                    if((args[j] = args[j + 2]) == NULL) break;
                }
                i --;       //�ض���һλ
            }
            else if(*args[i] == '>'){
                if(*(args[i] + 1) == '>'){  //>>
                    int fd = open(args[i + 1], O_RDWR | O_APPEND | O_CREAT, 0666);
                    if(fd < 0){
                        printf("cannot open file!\n");
                        continue;
                    }
                    EnQueue(&Qa, fd); //�����Qa
                }
                else {          //>
                    int fd = open(args[i + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
                    if(fd < 0){
                        printf("cannot open file!\n");
                        continue;
                    }
                    EnQueue(&Qw, fd);//�����Qw
                }
                int kk;
                for(kk = i; ; kk ++){     //��ָ��������ɾ��������ļ���
                    if((args[kk] = args[kk + 2]) == NULL) break;
                }
                i --;       //�ض���һλ
            }
        }
        int temppipe[2];
        if(!QueueEmpty(Qa) || !QueueEmpty(Qw)){ //��Ҫд�ļ�
            pipe(temppipe);
            dup2(temppipe[1], 1);   //��д���ܵ���    �����>�� >> ��ʱ����Ҫд�����ļ���������д��ܵ�������ٰѹܵ������ݶ�������ļ��С�
            close(temppipe[1]);
        }
        if(pipenum == 0)goto next;  //û�йܵ�
        int pipefd[128][2];
        //memset(pipefd, 0, sizeof(int) * 128 * 2);
        for(i = 0; i <= pipenum; i ++){     //pipenum + 1��
            if(i < pipenum)pipe(pipefd[i]); //pipenum��
            pid_t pid = fork();//pipenum + 1��
            if (pid < 0){
                printf("fork failed!\n");       //forkʧ��
                return(255);
            }
            else if(pid == 0)break; //�ӽ�������ѭ��
            //�����̼������ѭ��
        }
                /* �ӽ��� */
                if(i == 0){     //��һ���ӽ���
                    dup2(pipefd[0][1], 1);  //д���ܵ���
                    closeAll(i, pipefd);
                    execvp(*newcmd[i], newcmd[i]);
                    /* execvpʧ�� */
                    printf("no such instruction!\n");        //δ����ָ��
                    return 255;
                }
                else if(i == pipenum){  //���һ������
                    dup2(pipefd[pipenum - 1][0], 0);  //�ӹܵ��ж�
                    closeAll(i - 1, pipefd);
                    execvp(*newcmd[i], newcmd[i]);
                    /* execvpʧ�� */
                    printf("no such instruction!\n");        //δ����ָ��
                    return 255;
                }
                else if(i < pipenum){   //��������
                    dup2(pipefd[i - 1][0], 0);
                    dup2(pipefd[i][1], 1);
                    closeAll(i, pipefd);
                    execvp(*newcmd[i], newcmd[i]);
                    /* execvpʧ�� */
                    printf("no such instruction!\n");        //δ����ָ��
                    return 255;
                }
                else {              //������
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

        /* û���������� */
     next:   if (!args[pos])
            continue;

        /* �ڽ����� */
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

        /* �ⲿ���� */
        pid_t pid = fork();
        if (pid < 0){
            printf("fork failed!\n");       //forkʧ��
            continue;
        }
        else if (pid == 0) {
            /* �ӽ��� */
            execvp(args[pos], args);    //ptrmerge(args[pos], pipeargs[0])
            /* execvpʧ�� */
            printf("no such instruction!\n");        //δ����ָ��
            return 255;
        }
        /* ������ */
        wait(NULL);     //�ȴ��ӽ��̽���
        check:if(!QueueEmpty(Qa) || !QueueEmpty(Qw)){ //��Ҫд�ļ�
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
