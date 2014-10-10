
class Triangle
{

	protected:
		int size;
		int **array;
		void init();
		void destroy();


	public:
		Triangle(int size);
		~Triangle();
		void print();

};

