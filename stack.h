
class Node;

class Stack
{

	protected:
		int capacity;
		int size;
		Node** nodes;

	public:
		Stack();
		~Stack();
		void push(Node* n);
		Node* pop();
		const Node* top();
		int getSize();
};
