#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/buffer_head.h>
#include <linux/net.h>
#include <linux/vm_sockets.h>
#include <linux/in.h>
#include <linux/nsproxy.h>
#include <linux/errno.h>
#include "hypervfs.h"

#define PORT_NUM 5001
#define SOCKET_NUM 4
#define MAX_SOCK_BUF (1024*1024)

static struct socket *sockets[SOCKET_NUM] = { NULL };
static struct socket *change_socket = NULL;

static struct socket *hypervfs_connect_socket(void)
{
	int err;
	struct socket *csocket;
	struct sockaddr_vm addr = { 0 };

	addr.svm_family = AF_VSOCK;
	addr.svm_port = PORT_NUM;
	addr.svm_cid = VMADDR_CID_HOST;

	err = __sock_create(
		current->nsproxy->net_ns,
		PF_VSOCK,
		SOCK_STREAM,
		PF_VSOCK,
		&csocket,
		1
	);

	if (err) {
		pr_err("%s (%d): problem creating socket\n - %d", __func__, task_pid_nr(current), err);
		return ERR_PTR(err);
	}

	err = csocket->ops->connect(
		csocket,
		(struct sockaddr*)&addr,
		sizeof(addr),
		0
	);
	

	if (err < 0) {
		pr_err("%s (%d): problem connecting socket to %d\n",  __func__, task_pid_nr(current), addr.svm_port);
		sock_release(csocket);
		return ERR_PTR(err);
	}

	return csocket;
}

int hypervfs_op_connect()
{
	
	int i;
	for (i = 0; i < SOCKET_NUM; i++) {
		sockets[i] = hypervfs_connect_socket();
	}

	// one more socket for change detection
	change_socket = hypervfs_connect_socket();

	return 0;
}