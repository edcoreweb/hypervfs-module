#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/buffer_head.h>
#include <linux/net.h>
#include <linux/vm_sockets.h>
#include <linux/in.h>
#include <linux/nsproxy.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/slab.h>
#include "hypervfs.h"

#define PORT_NUM 5001
#define SOCKET_NUM 4
#define MAX_SOCK_BUF (1024*1024)

struct hypervfs_socket {
	struct socket *csocket;
	struct list_head list;
};

// static DEFINE_SPINLOCK(sockets_lock);
static LIST_HEAD(sockets);
static struct hypervfs_socket *change_socket = NULL;

static struct hypervfs_socket *hypervfs_socket_init(struct socket *csocket)
{
	struct hypervfs_socket *socket;
	socket = (struct hypervfs_socket *)kmalloc(sizeof(socket), GFP_KERNEL);

	if (!socket)
		return ERR_PTR(-ENOMEM);

	socket->csocket = csocket;

	return socket;
}

static void hypervfs_socket_destroy(struct hypervfs_socket *socket)
{
	sock_release(socket->csocket);
	// kfree(socket);
}

static struct hypervfs_socket *hypervfs_connect_socket(void)
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

	return hypervfs_socket_init(csocket);
}

int hypervfs_op_connect()
{
	int i;
	int err = 0; 

	for (i = 0; i < SOCKET_NUM; i++) {
		struct hypervfs_socket *socket = hypervfs_connect_socket();
		if (IS_ERR(socket)) {
			err = PTR_ERR(socket);
			goto out;
		}


		list_add(&socket->list, &sockets);
	}

	// one more socket for change detection
	change_socket = hypervfs_connect_socket();
	if (IS_ERR(change_socket)) {
		err = PTR_ERR(change_socket);
		goto out;
	}

out:
	hypervfs_op_close();
	return err;
}

void hypervfs_op_close(void)
{
	struct hypervfs_socket *pos, *n;

	list_for_each_entry_safe(pos, n, &sockets, list) {
		list_del(&pos->list);
		hypervfs_socket_destroy(pos);
	}

	if (change_socket) {
		hypervfs_socket_destroy(change_socket);
	}
}