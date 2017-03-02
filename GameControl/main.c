#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DEBUG 1 //Plus tard : à rentrer en argument au lancement du programme

int DeplacementPiece(char *move)
{
    //PLACEHOLDER, Voir déplacement plus tard
    sleep(2);
    return 0;
}
void Debug_AffichVar(char *variable, int longueur)
//affiche tous les caractères de la chaine (en caractère, puis avec le code ascii correspondant)
{
    printf("\n\ncontenu de la chaine : \n");
    int a =0;
    while(a < longueur)
    {
        printf("%c %d\n", variable[a], variable[a]);
        a++;
    }
    printf("\n");
}

int main()
{
    int PipeSortante[2], PipeEntrante[2];
    char message[10], move[5], ordre[20];
    char settings = DEBUG;

    //déclaration des pipes
    pipe(PipeSortante);
    pipe(PipeEntrante);

    pid_t childstatus;
    int status;

    //Création du processus enfant : le programme se divise, fork
    childstatus = fork();

    if(childstatus == 0) //child
    {
        //fermeture des pipes inutiles au child
        close(PipeEntrante[0]);
        close(PipeSortante[1]);

        //on remplace  stdin par la pipe sortante du parent pour envoyer des commandes
        fclose(stdin);
        dup2(PipeSortante[0], STDIN_FILENO);
        printf("Connexion au parent établie, fermeture de stdout\n");

        //on ferme stdout pour écrire dans un fichier, que le parent lira ensuite
        fclose(stdout);
        stdout = fopen("./Stdout_Child.txt", "w");
        dup2(STDOUT_FILENO, PipeEntrante[1]);

        system("gnuchess -x"); //on lance gnuchess en mode xboard (controle par commande)

        return 0;
    }
    else //parent
    {

        //Déclaration variables parent
        FILE *SortieGnuchessFils;
        int nmbrcoup = 1;
        char continuer = 1, loop = 1;
        char TestCaract = 0; int posCaract = 0;


        //Ouverture fichier
        SortieGnuchessFils = fopen("./Stdout_Child.txt", "r");

        //fermeture des pipes inutiles au parent
        close(PipeEntrante[1]);
        close(PipeSortante[0]);


        //initialisation des chaines de caractères
        memset(message, '\0', 10);
        memset(ordre, '\0', 20);
        memset(move, '\0', 5);

        //dup2(STDIN_FILENO, PipeSortante[1]);


        sleep(2);
        write(PipeSortante[1], "xboard \n", strlen("Xboard \n"));
        sleep(1);
        write(PipeSortante[1], "new \n", strlen("Xboard \n"));
        sleep(1);
        printf("waiting\n"); // le programme est prêt à recevoir des ordres

        while(continuer == 1)
        {
            loop = 1;

            scanf("%s", move);
            sprintf(ordre, "%s\n", move); //si le déplacement est invalide / la syntaxe n'est pas respectée,
                                          //gnuchess dira que le déplacement est invalide : on n'a pas à vérifier l'ordre.
            write(PipeSortante[1], ordre, strlen(ordre));

            sleep(2);

            do
            {
                fseek(SortieGnuchessFils, -8, SEEK_END);
                fread(message, 4, 1, SortieGnuchessFils);
                if(message[0] != (nmbrcoup % 10)+48) //Si le programme n'a pas répondu , les derniers caractères
                {                                    //sont de la forme "1. "ordre""
                    loop = 0;
                    printf("sortie");
                }
                //printf("Message lu = %s\nOrdre = %s\nStrccmp = %d", message, move, strncmp(message, move, 4));
                if (settings ==1) {
                printf("%c -> %d ", message[0], nmbrcoup % 10);
                printf("Pas de réponse de Gnuchess pour le moment\n");
                }
                sleep(1);

            }while(loop == 1); //Tant que le programme n'a pas répondu, on attend

            //Ré-initialisation des chaines de caractères
            memset(message, '\0', 10);
            memset(ordre, '\0', 20);
            memset(move, '\0', 5);

            //Relecture pour obtenir les 4 derniers caractères du fichier : on regarde le déplacement gnuchess
            fseek(SortieGnuchessFils, -11, SEEK_END);
            fread(message, 4, 1, SortieGnuchessFils);

            if(message[0] == 'm') //Correspond au m dans "Invalid move"
            {
                printf("Le déplacement est invalide, réessayer.\n");
            }

            else { //déplacement accepté par gnuchess

                TestCaract = 0;
                posCaract = 0;

                if(settings == 1) {Debug_AffichVar(message, 10);}

                DeplacementPiece(move); // on déplace d'abord la pièce : le programme est mis en pause pendant
                                        //le déplacement (dans la fonction DeplacementPiece()

                while(TestCaract != 32) //tant que le caractère lu n'est pas un espace (32 = code ascii de l'espace)
                {
                    posCaract--;
                    fseek(SortieGnuchessFils, posCaract, SEEK_END); //alors on regarde le caractère d'avant
                    fread(&TestCaract, 1, 1, SortieGnuchessFils);
                    if (settings == 1)
                    {
                    printf("Caractere lu : %d  ", posCaract);
                    printf("%c  ", TestCaract);
                    printf("%d\n", TestCaract);
                    }
                    //sleep(1);
                }

                fseek(SortieGnuchessFils, posCaract, SEEK_END); //on récupère le déplacement exact de gnuchess
                fread(message, 1, -posCaract, SortieGnuchessFils);

                if(settings == 1) {Debug_AffichVar(message, 10);}

                printf("Déplacement adversaire : %s\n", message);

                DeplacementPiece(message); //On peut maintenant déplacer la pièce déplacée par gnuchess
            }

            nmbrcoup++; //A réfléchir plus tard : est-ce qu'on compte les coups non réalisés (syntaxe, etc)?
        }

        sleep(2);
        write(PipeSortante[1], "quit\n", strlen("quit\n"));

        waitpid(childstatus, &status, 0);
        return 0;
    }


}
