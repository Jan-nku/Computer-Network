#include"Table.h"

//存储网卡的双IP、双掩码
DWORD ip[2], mask[2];
BYTE mac[6];
Buffer buffer[BufferSize];

void routeItem::printItem() {
	in_addr addr;
	addr.s_addr = mask;
	printf("掩码:%s\t", inet_ntoa(addr));
	addr.s_addr = net;
	printf("目的网络:%s\t", inet_ntoa(addr));
	addr.s_addr = nextip;
	printf("下一跳:%s\t", inet_ntoa(addr));
	printf("类型:%d\n", type);
};


routeTable::routeTable() {
	head = new routeItem;
	tail = new routeItem;
	head->nextItem = tail;
	tail->type = -1;
	num = 0;
	for (int i = 0; i < 2; i++) {
		routeItem* temp = new routeItem;
		temp->net = ip[i] & mask[i];
		temp->mask = mask[i];
		temp->nextip = 0;
		temp->type = 0;
		this->add(temp);
	}
}

void routeTable::add(routeItem* item){
	routeItem* tmp;
	if (item->type == 0) {
		item->nextItem = head->nextItem;
		head->nextItem = item;
	}
	else {
		tmp = head->nextItem;
		while (tmp->nextItem->type == 0 || item->mask < tmp->nextItem->mask) {
			if (tmp->nextItem == tail) {
				break;
			}
			tmp = tmp->nextItem;
		}
		item->nextItem = tmp->nextItem;
		tmp->nextItem = item;
	}
	num++;
}

void routeTable::remove(int index){
	if (index > num) {
		printf("请删除索引正确的表项\n");
		return;
	}
	routeItem* tmp = head;
	while (--index) {
		tmp = tmp->nextItem;
	}
	if (tmp->nextItem->type == 0) {
		printf("直接投递项不可删除\n");
		return;
	}
	routeItem* item = tmp->nextItem;
	tmp->nextItem = tmp->nextItem->nextItem;
	free(item);
	num--;
	printf("删除成功\n");
}

void routeTable::printTable() {
	for (routeItem* tmp = head->nextItem; tmp != tail; tmp = tmp->nextItem) {
		tmp->printItem();
	}
	return;
}

DWORD routeTable::lookup(DWORD ip) {
	for (routeItem* tmp = head->nextItem; tmp != tail; tmp = tmp->nextItem) {
		if ((ip & tmp->mask) == tmp->net) {
			return tmp->nextip;
		}
	}
	return -1;
}

void routeTable::printNum() {
	printf("路由表条数：%d", num);
}

arpTable arptable[50];
int arpTable::num = 0;

void arpTable::insert(DWORD ip, BYTE mac[6]) {
	if (num + 1 == 50) {
		cout << "arp表已满，无法插入" << endl;
		return;
	}
	arptable[num].ip = ip;
	memcpy(arptable[num].mac, mac, 6);
	num++;
}

int arpTable::lookup(DWORD ip, BYTE mac[6]) {
	for (int index = 0; index < num; index++) {
		if (arptable[index].ip == ip) {
			memcpy(mac, arptable[index].mac, 6);
			return index;
		}
	}
	return -1;
}

void arpTable::printArp() {
	if (num == 0) {
		cout << "当前arpTable为空" << endl;
		return;
	}
	cout << "打印arpTable" << endl;
	for (int i = 0; i < num; i++) {
		in_addr addr;
		addr.s_addr = arptable[i].ip;
		printf("IP：%s\t", inet_ntoa(addr));
		printf("MAC:");
		for (int i = 0; i < 5; i++) {
			printf("%02X-", arptable[i].mac[i]);
		}
		printf("%02X\n", arptable[i].mac[5]);
	}
}
