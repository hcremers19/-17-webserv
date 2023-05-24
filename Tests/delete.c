#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1" // Adresse IP du serveur HTTP

void send_delete_method(const char* fileName, const char* port)
{
	int socket_fd;
	struct sockaddr_in server_addr;

	// Création du socket
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1)
	{
		perror("Erreur lors de la création du socket");
		exit(EXIT_FAILURE);
	}

	// Configuration de l'adresse du serveur
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(port));
	if (inet_pton(AF_INET, SERVER_IP, &(server_addr.sin_addr)) <= 0)
	{
		perror("Adresse du serveur invalide");
		exit(EXIT_FAILURE);
	}

	// Connexion au serveur
	if (connect(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		perror("Erreur lors de la connexion au serveur");
		exit(EXIT_FAILURE);
	}

	// Construction de la requête DELETE
	char request[1000];
	sprintf(request, "DELETE ");
	strcat(request, fileName);
	strcat(request, " HTTP/1.1\r\n");
	strcat(request, "Host: localhost:");
	strcat(request, port);
	strcat(request, "\r\n");
	strcat(request, "Connection: close\r\n");
	strcat(request, "\r\n");

	// Envoi de la requête DELETE
	if (send(socket_fd, request, strlen(request), 0) == -1)
	{
		perror("Erreur lors de l'envoi de la requête");
		exit(EXIT_FAILURE);
	}

	// Réception de la réponse du serveur
	char response[10000];
	ssize_t num_bytes;
	while ((num_bytes = recv(socket_fd, response, sizeof(response) - 1, 0)) > 0)
	{
		response[num_bytes] = '\0';
		printf("%s", response);
	}
	if (num_bytes == -1)
	{
		perror("Erreur lors de la réception de la réponse");
		exit(EXIT_FAILURE);
	}

	// Fermeture du socket
	close(socket_fd);
}

int	main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("Usage: ./a.out <fileName> <port>");
		return (1);
	}

	send_delete_method(argv[1], argv[2]);

	return (0);
}
