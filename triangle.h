enum Direction
{
	TOP_LEFT,
	TOP_RIGHT,
	LEFT,
	RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};

class Triangle
{

	protected:
		int size;
		int **array;
		void init();
		void destroy();
                int move(int dx, int dy);
                int emptyX;
                int emptyY;

	public:
		Triangle(int size);
		~Triangle();
		void print();
		void fill();
                int move(Direction where);
                Direction oppositeDirection(Direction dir);
                void randomStep();
                bool isSorted();
                int getDistanceX();

};

