#include <iostream>

#include "msg_list.h"

msg_list::msg_list(){
	front = back = NULL;
}

void msg_list::display() {
	msg_list_node *tmp;  int i;
	
	if(front==NULL) {
		cout << "** List is empty. **\n";
		return;
	}
	
	tmp = front;  i = 1;
	while(tmp!=NULL) {
		cout << "Datagram " << i++ << ":  \n";
		(tmp->d)->display();
		cout << "\n";
		tmp = tmp->next;
	}
	
}

void msg_list::append(datagram *x) {
	msg_list_node* tmp = new msg_list_node;
	tmp->next = NULL;
	tmp->d = x;
    
	if(front==NULL)
        front = tmp;
	else
        back->next = tmp;
	back = tmp;
}

datagram* msg_list::returnFront(){
    if (front == NULL) return NULL;
    msg_list_node* n = new msg_list_node;
    datagram* d = front->d;
    n = front;
    
    
    front = n->next;
    delete n;
    return d;
}

void msg_list::deleteList(){
    
    msg_list_node* tmp;
    tmp = front;

    
    while(front != NULL){
        if (tmp->next == NULL){
            delete tmp->d;
            delete tmp;
            front = NULL;
            back = NULL;
            tmp = NULL;

        }
        else{
            front = front->next;
            delete tmp->d;
            delete tmp;
            tmp = front;
        }
    }
    
    
}
