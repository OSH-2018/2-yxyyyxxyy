/*******************************************************************
*   ֧�������Ŀո���롣֧�� | ������/�޿ո������� | �� || ���Ƕ� || ����Ӧ����֧�ֳ��ո� | ||֮��ķָ�����ָ�ͷ���Լӿո�
*   ֧�ֹܵ���������֧�����128���ܵ�
*   ֧�����û�������
*
*******************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

//#define splitDebug

typedef enum bool{
    false = 0, true = 1
}bool;

void closeAll(int pipenum, int pipefd[][2]){    //�ص�0-pipenum�Ķ�ά����ܵ���
    for(int i = 0; i <= pipenum; i ++){
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
    while (1) {
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
                    if(*args[i+1] == '|'){      // | ��û�пո����� �ų� ||
                        if(*(args[i+1] + 1) != '|'){    //���� ||
                            args[i+2] = args[i+1];
                            args[++i] = "|";
                        }
                        else {     // ��||
                            args[i+2] = args[i+1] + 1;
                            args[++i] = "||";
                        }
                        goto loop;
                    }
                    break;
                }
                else if(*args[i+1] == '|'){
                    *args[i+1] = '\0';
                    if(*(args[i+1] + 1) != '|'){
                        args[i+2] = args[i+1];
                        args[++i] = "|";        //���� | ǰû�пո����� �ų� ||
                    }
                    else {
                        args[i+2] = args[i+1] + 1;
                        args[++i] = "||";        // ||
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

        for(i = 0; args[i]; i ++){      //ͳ�ƹܵ��� ls -l | wc NULL
            if(*args[i] == '|'){
                pipenum ++;
                newcmd[pipenum] = args + i + 1;
                args[i] = NULL;     //��ָ��ܵ��� | ת��Ϊ NULL
            }
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
    }
}
