#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DEBUG 1 //Plus tard : à rentrer en argument au lancement du programme
#define TEXTE 0 //Pour l'instant, on reste en mode saisie par texte
#define JAVA_ORDER NULL //commande à envoyer au cmd pour lancer le programme
                        //A compléter plus tard

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
void GetVoiceOrder(int Sortante, int Entrante, char *chaine)
{
    //variables
    char continuer = 1;
    char CaracLu = 0;
    int position = 0;

    write(Sortante, "1\n", 2); //On envoie au programme que l'on peut arrêter

    usleep(50000); //On attend 50 ms
    while(continuer == 1) //On attend tant que le programme n'a par répondu
    {
        usleep(500000); //On attend 500 ms
        lseek(Entrante, -1, SEEK_END);
        read(Entrante, &CaracLu, 1);
        if(CaracLu != 49)  //Si le caractère lu est différent du retour à la ligne envoyé par l'appli java
        {                  //A verifier avec le prog de maxence mais devrait fonctionner
            continuer = 0;
        }
    }
    continuer = 1;

    //On cherche maintenant la position
    while(continuer == 1);
    {
        position--;
        lseek(Entrante, position, SEEK_END);
        read(Entrante, &CaracLu, 1);
        if(CaracLu != 10)
        {
            continuer = 0;
        }
    }
    //Mainenant qu'on a la position, on peut récuperer la chaine entière
    read(Entrante, chaine, 10); //on écrit directement dans la chaine fournie en argument
    return ;
}

int main()
{
    int PipeSortante[2], PipeEntrante[2]; //Préparation des pipes de communiquation
    int PipeSortanteJ[2], PipeEntranteJ[2];
    char message[10], move[5], ordre[20];
    char settings[3];
    settings[0] = DEBUG; //0 = fonctionnement normal, 1 = mode debug / Verbose
    settings[1] = TEXTE; //0 = saisie par texte, 1 = saisie par voix (fonctionnement normal); A REMPLACER PLUS TARD

    //déclaration des pipes
    pipe(PipeSortante);
    pipe(PipeEntrante);
    pipe(PipeEntranteJ);
    pipe(PipeSortanteJ);

    pid_t childstatus[3];
    int status;

    //Création du processus enfant : le programme se divise, fork
    childstatus[1] = fork();

    if(childstatus[1] != 0) //Pour le père seulement
    {
        childstatus[2] = fork(); //On se redivise encore pouvoir communiquer avec l'appli java
    }

    if(childstatus[1] == 0) //1er child
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
        //Le processus fils sera mis en "pause" pendant l'execution de gnuchess. Lorsque gnuchess se fermera,
        //le processus fils se terminera.

        return 0;
    }
    else if(childstatus[2] == 0) //Deuxième proc fils, utilisé pour communiquer avec l'appli de reco de voix
    {
        //EN TRAVAUX
        //Fermeture des pipes inutiles
        close(PipeEntranteJ[0]);
        close(PipeSortanteJ[1]);

        //Encore une fois, stdin sera remplacé par la pipe venant du parent
        fclose(stdin);
        dup2(PipeSortanteJ[0], STDIN_FILENO);
        printf("Connexion au parent établie, fermeture de stdout (java)\n");

        fclose(stdout);
        dup2(STDOUT_FILENO, PipeSortanteJ[1]); //le processus ecrira dans la pipe au lieu de stdout

        system("JAVA_ORDER"); //Lancement de l'applcation java


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
        close(PipeEntranteJ[1]);
        close(PipeSortanteJ[0]);


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

            //Ré-initialisation des chaines de caractères
            memset(message, '\0', 10);
            memset(ordre, '\0', 20);
            memset(move, '\0', 5);


            if(settings[1] == 0) //Mode textuel uniquement
            {
                scanf("%s", move);
                sprintf(ordre, "%s\n", move);
            }
            else
            {
                GetVoiceOrder(PipeSortanteJ[1], PipeEntranteJ[0], ordre);
            }

            //si le déplacement est invalide / la syntaxe n'est pas respectée,
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

                if (settings[0] ==1) {
                printf("%c -> %d ", message[0], nmbrcoup % 10);
                printf("Pas de réponse de Gnuchess pour le moment\n");
                }
                sleep(1);

            }while(loop == 1); //Tant que le programme n'a pas répondu, on attend
            printf("\n");

            //Ré-ré-initialisation des chaines de caractères (encore ...)
            memset(message, '\0', 10);
            memset(ordre, '\0', 20);
            memset(move, '\0', 5);

            //Relecture pour obtenir les 4 derniers caractères du fichier : on regarde le déplacement gnuchess
            fseek(SortieGnuchessFils, -11, SEEK_END);
            fread(message, 4, 1, SortieGnuchessFils);

            if(message[0] == 'm') //Correspond au m dans "Invalid move"
            {
                printf("Le déplacement est invalide, réessayer.\n");
                printf("%s", message);
            }

            else { //déplacement accepté par gnuchess

                TestCaract = 0;
                posCaract = 0;

                if(settings[0] == 1) {Debug_AffichVar(message, 10);}

                DeplacementPiece(move); // on déplace d'abord la pièce : le programme est mis en pause pendant
                                        //le déplacement (dans la fonction DeplacementPiece()

                while(TestCaract != 32) //tant que le caractère lu n'est pas un espace (32 = code ascii de l'espace)
                {
                    posCaract--;
                    fseek(SortieGnuchessFils, posCaract, SEEK_END); //alors on regarde le caractère d'avant
                    fread(&TestCaract, 1, 1, SortieGnuchessFils);
                    if (settings[0] == 1)
                    {
                    printf("Caractere lu : %d  ", posCaract);
                    printf("%c  ", TestCaract);
                    printf("%d\n", TestCaract);
                    }
                    //sleep(1);
                }

                fseek(SortieGnuchessFils, posCaract, SEEK_END); //on récupère le déplacement exact de gnuchess
                fread(message, 1, -posCaract, SortieGnuchessFils);

                if(settings[0] == 1) {Debug_AffichVar(message, 10);}

                printf("Déplacement adversaire : %s\n", message);

                DeplacementPiece(message); //On peut maintenant déplacer la pièce déplacée par gnuchess
                nmbrcoup++;
            }

        }

        sleep(2);
        write(PipeSortante[1], "quit\n", strlen("quit\n"));
        write(PipeSortanteJ[1], "0\n", 2);

        waitpid(childstatus[1], &status, 0);
        waitpid(childstatus[2], &status, 0);

        return 0;
    }


}
