
class Node;

class Stack
{

	protected:
		int capacity;
		int size;
		Node** nodes;
		void expand();
		void destroy();
                
	public:
		Stack();
		~Stack();
		void push(Node* n);
		Node* pop();
		const Node* top();
		int getSize();
};
