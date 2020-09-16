#include	"unp.h"

void str_cli(FILE *fp, int sockfd)
{
	int maxfdp1, stdineof;
	fd_set rest;
	char buf[MAXLINE];
	int n;
	long arg, result;

	stdineof = 0;
	FD_ZERO(&rest);
	result = 0;
	for ( ; ; ) {
		if ( stdineof == 0 )
			FD_SET(fileno(fp), &rest);
		FD_SET(sockfd, &rest);
		maxfdp1 = max(fileno(fp), sockfd) + 1;
		Select(maxfdp1, &rest, NULL, NULL, NULL);

		if ( FD_ISSET(sockfd, &rest) ) {
			n = Readn(sockfd, &arg, sizeof(arg));
			result += arg;
			printf("<%ld>\n", result);
			if ( result > 31 ) {
				printf("quit2\n");
				printf("quit3\n");
				return ;
			}
		}

		if ( FD_ISSET(fileno(fp), &rest) ) {
			A:
			n = Read(fileno(fp), buf, MAXLINE);
			if (sscanf(buf, "%ld", &arg) != 1) {
				printf("invalid input: %s", buf);
				goto A;
			}
			if ( arg < 0 || 10 < arg ) {
				printf("send a numbers between 1 to 9\n");
				goto A;
			}
			result += arg;
			printf("<%ld>\n", result);
			if ( result > 31 ) {
				Writen(sockfd, &arg, sizeof(arg));
				printf("quit1\n");
				stdineof = 1;
				Shutdown(sockfd, SHUT_WR);
				FD_CLR(fileno(fp), &rest);
				continue;
			}
			Writen(sockfd, &arg, sizeof(arg));
		}
	}
}

int
main(int argc, char **argv)
{
	int					sockfd, n;
	struct sockaddr_in	servaddr;
	char buff[MAXLINE];

	if (argc != 3)
		err_quit("usage: a.out <IPaddress>");

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(atoi(argv[2]));	/* daytime server */
	Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));
	printf("[connected to %s:%d]\n", Inet_ntop(AF_INET, &servaddr.sin_addr, buff, sizeof(buff)), ntohs(servaddr.sin_port));
	str_cli(stdin, sockfd);

	exit(0);
}
