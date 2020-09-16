#include	"unp.h"
#include	<time.h>

int
main(int argc, char **argv)
{
	int i, maxi, listenfd, connfd, sockfd, maxfd;
	int nready;
	ssize_t n;
	char buf[MAXLINE];
	socklen_t clilen;
	int client[FD_SETSIZE];
	struct sockaddr_in	servaddr, cliaddr;
	fd_set allset, rset;
	FILE *fp = stdin;
	int stdineof = 0;
	long arg, result;

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));	/* daytime server */

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	maxfd = listenfd;
	maxi = -1;
	for ( i = 0; i < FD_SETSIZE; i++ )
		client[i] = -1;
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);
	FD_SET(fileno(fp), &allset);
	result = 0;
	for ( ; ; ) {
		rset = allset;
		nready = Select(maxfd + 1, &rset, NULL, NULL, NULL);
		if ( FD_ISSET(listenfd, &rset) ) {
			printf("[waiting for an opponent]\n");
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
			printf("[connected to %s:%d]\n", Inet_ntop(AF_INET, &cliaddr.sin_addr, buf, sizeof(buf)), ntohs(cliaddr.sin_port));
			for ( i = 0; i < FD_SETSIZE; i++ )
				if ( client[i] < 0 ) {
					client[i] = connfd;
					break;
				}
			if ( i == FD_SETSIZE )
				err_quit("too many clients");
			FD_SET(connfd, &allset);
			if ( connfd > maxfd )
				maxfd = connfd;
			if ( i > maxi )
				maxi = i;
	
			if ( --nready <= 0 )
				continue;
		}
		
		for ( i = 0; i <= maxi; i++ ) {
			if ( (sockfd = client[i]) < 0 )
				continue;
			if ( FD_ISSET(sockfd, &rset) ) {
				n = Readn(connfd, &arg, sizeof(arg));
				result += arg;
				printf("<%ld>\n", result);
				if ( result > 31 ) {
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i] = -1;
					result = 0;
				}
				if ( --nready <= 0 )
					break;
			}
			if ( FD_ISSET(fileno(fp), &rset) ) {
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
					stdineof = 1;
					Shutdown(sockfd, SHUT_WR);
					FD_CLR(fileno(fp), &rset);
					continue;
				}
				Writen(sockfd, &arg, sizeof(arg));
				if ( --nready <= 0 )
					break;
			}
		}
	}
}
