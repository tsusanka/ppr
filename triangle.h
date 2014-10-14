
class Triangle
{

	protected:
		int size;
		int **array;
		void init();
		void destroy();
		int *empty;


	public:
		Triangle(int size);
		~Triangle();
		void print();
		void fill();

};

