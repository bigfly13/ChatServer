#include<iostream>
#include<string>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<map>
#include<unistd.h>

// 最大连接数
const int MAX_CONN = 1024;

// 客户端信息
struct Client
{
	int socketfd;
	std::string name;
};

int main()
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(9999);

	int op = 1;
	int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op));
	if (ret != 0)
	{
		perror("setsockopt");
		return -1;
	}
	//printf("setsockopt success\n");

	ret = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret != 0)
	{
		perror("bind");
		return -1;
	}
	//printf("bind success\n");

	ret = listen(listenfd, 5);
	if (ret != 0)
	{
		perror("listen");
		return -1;
	}
	//printf("listen success\n");

	int epfd = epoll_create(5);

	struct epoll_event listen_event;
	listen_event.data.fd = listenfd;
	listen_event.events = EPOLLIN;
	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &listen_event);
	if (ret != 0)
	{
		perror("epoll_ctl");
		return -1;
	}
	//printf("epoll_ctl success\n");

	// 保存客户端信息
	std::map<int, Client> clients;
	while (1)
	{
		struct epoll_event events[MAX_CONN];
		printf("start wait\n");
		int n = epoll_wait(epfd, events, MAX_CONN, -1);
		if (n == -1)
		{
			perror("epoll_wait\n");
			break;
		}
		else
		{
			printf("have events\n");
		}

		for (int i = 0; i < n; i++)
		{
			//printf("i = %d\n", i);
			int current_fd = events[i].data.fd;
			if (current_fd == listenfd)
			{
				struct sockaddr_in client_addr;
				socklen_t len = sizeof(client_addr);
				//printf("wait accept\n");
				int conn_fd = accept(listenfd, (struct sockaddr*)&client_addr, &len);
				//printf("have accept\n");
				struct epoll_event client_event;
				client_event.data.fd = conn_fd;
				client_event.events = EPOLLIN;
				epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &client_event);
				
				printf("%s已连接\n", inet_ntoa(client_addr.sin_addr));

				Client client;
				client.socketfd = conn_fd;
				client.name = "";
				clients[conn_fd] = client;
			}
			else
			{
				char buf[1024] = {};
				int read_num = read(current_fd, buf, sizeof(buf));
				if (read_num < 0)
				{
					break;
				}
				else if (read_num == 0)
				{
					close(current_fd);
					epoll_ctl(epfd, EPOLL_CTL_DEL, current_fd, 0);
					clients.erase(current_fd);
				}
				else
				{
					std::string msg(buf, read_num);

					if (clients[current_fd].name == "")
					{
						clients[current_fd].name = msg;
					}
					else
					{
						std::string name = clients[current_fd].name;

						for (auto& c : clients)
						{
							if (c.first != current_fd)
							{
								write(c.first, ('[' + name + ']' + ": " + msg).c_str(), name.size() + msg.size() + 4);
							}
						}
					}
				}
			}
		}
	}
	close(epfd);
	close(listenfd);
}