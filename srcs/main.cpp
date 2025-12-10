#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include "../incs/Server.hpp"

// Instance globale du serveur
Server* serverInstance = NULL;

// Gestion des signaux
void handleSignal(int signal)
{
    const char* signalName;
    if (signal == CTRL_C)
        signalName = "SIGINT (ctrl + c)";
    else if (signal == CTRL_Z)
        signalName = "SIGTSTP (ctrl + z)";
    else
        signalName = "Unknown";

    std::cout << "\nSignal " << signalName << " reçu, fermeture du serveur..." << std::endl;

    if (serverInstance)
    {
        serverInstance->shutdown();
        serverInstance = NULL;
    }
    exit(0);
}

int main(int argc, char **argv)
{
    // Vérification des arguments
    if (argc != THREE_ARGMNTS)
    {
        std::cerr << "Usage: ./micro_irc <port> <password>" << std::endl;
        return EXIT_FAILURE;
    }

    // Conversion du port
    char *endptr;
    long port = std::strtol(argv[PORT_ARG_INDEX], &endptr, 10);

    // Port invalide
    if (*endptr != '\0' || port <= 0 || port > MAX_UINT16_BITS)
    {
        std::cerr << "Port invalide. Veuillez spécifier un port entre 1 et 65535." << std::endl;
        return EXIT_FAILURE;
    }

    // Mot de passe
    std::string password(argv[2]);

    // Configuration des signaux
    struct sigaction sa;
    sa.sa_handler = handleSignal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(CTRL_C, &sa, NULL) == FAILURE || sigaction(CTRL_Z, &sa, NULL) == FAILURE)
    {
        std::cerr << "Erreur lors de la configuration des signaux." << std::endl;
        return EXIT_FAILURE;
    }

    // Démarrage du serveur
    try {
        std::cout << "\033[1;34m\n";
        std::cout << "====================================================================================\n";
        std::cout << " * --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- *  \n";
        std::cout << "====================================================================================\n";
        std::cout << "\033[1;36m\n";                                                                           
        std::cout << "                    :-:::-=-::::::           .::::   .=*%@@@@#+            ..          \n";
        std::cout << "    +%*==++**#%*+: :%@@@@@@@%%%%#*          +@@%*: *@@@@@@@@@@@:       -*%#%#.        \n";
        std::cout << "    +@@@@*+==-:.    =*%@@@@@@#+==-           @@@@=.*@@@@@@*::=%@.     *@@*##@@-       \n";
        std::cout << "    .@@@*              .@@@*                =@@@*  .%++@@@-  -%@#    *@@*%**#@%       \n";
        std::cout << "    .@@%*  :-=*+-      :@@@+                *%@@-     ##@@==%@@@@   =@@%@. :%%*       \n";
        std::cout << "    .@@@@@@@@%*=-      =@@@=                +#@@.     @@@@@@@%##:   #@@%*   .-        \n";
        std::cout << "     @@@*+:.           -@@@-                +%@%      @@@@@@*-.     %@@@:             \n";
        std::cout << "     *@@+-             *=@@:                =@@%.    .%@@@+@%=.      +@@%---=*%#-     \n";
        std::cout << "     =%@*.             *.##.  -+++=+++*+.:  #@@@@=    *@@@  =#@@@+     :=**%@@%*-     \n";
        std::cout << "     .-:.              : #*.  :::::--==-     =%%=     .#@@     :==.                   \n";
        std::cout << "                                                         :=.                        \n";
        std::cout << "\n\033[1;34m";
        std::cout << "====================================================================================\n";
        std::cout << " * --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- ** --@-- *  \n";
        std::cout << "====================================================================================\n\n";
        std::cout << "\033[1;32m";
        std::cout << "🚀 Server started successfully!\n";
        std::cout << "\033[1;34m";
        std::cout << "------------------------------------------------------------------------------------\n";
        std::cout << "\033[1;33m";
        std::cout << "🔌 Port     : \033[1;37m" << port << "\n";
        std::cout << "\033[1;33m" << "🔑 Password : \033[1;37m" << password << "\n";
        std::cout << "\033[1;34m";
        std::cout << "====================================================================================\n";
        std::cout << "\033[0m";

        Server server(static_cast<unsigned short>(port), password);
        serverInstance = &server;
        server.run();
    }

    // Exceptions
    catch (const std::exception &e)
    {
        std::cerr << "Erreur du serveur : " << e.what() << std::endl;
        if (serverInstance)
        {
            delete serverInstance;
            serverInstance = NULL;
        }
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
