//============================================================================
// Name        : findsquaresr.cpp
// Author      : Timothy Herchen
// Version     : 1.0.0
// Copyright   : N/A
// Description : Efficient (?) Algorithm for Square-Finding in a Rectangular Lattice
//============================================================================

#include <iostream>
#include <random>
#include <cmath>
#include <ctime>

using namespace std;

/*
 * To-do List:
 * Precisely calculate time and memory complexity
 * Add ability to test "fringe" cases
 * Add compatibility for GPU / multi-core processing
 * Add standard deviation, etc. statistics to program
 */

struct Point {
	/* Point is the central data type of the Computation.
	 * Parameters: (int x, int y) is the position of a point in Euclidean space.
	 */
	int x;
	int y;
};

Point nullp;				// Point with coordinates (-1,-1) to serve as a fast nullptr

struct Square {
	/* Square is the storage of the solution.
	 * Parameters: (Point v1, Point v2, Point v3, Point v4) are the vertices of the square.
	 */
	Point v1,v2,v3,v4;
};

Square nullsq;				// Square with vertices (nullp, nullp, nullp, nullp) to serve as a fast nullptr

const int sz = 10; 		// Size of grid
const long int trials = 10000000000;	// Number of trials on the grid

bool grid[sz][sz]; 			// Boolean array, storing state of each point: false = blank, true = filled in

Point points[sz*sz/4]; 		// Point array, storing all point positions
int pointpos = 0; 			// Index of last meaningful point in array points

default_random_engine generator;							// Random engine: mt19937
uniform_int_distribution<int> distribution(0,sz);			// Distribution over integers 0, 1,..., sz-1

int randompos() {
	/* Random number generation. The generators are used to pick points to color in from the grid.
	 * Mersenne Twister (mt19937) Generator, generating over integers 0, 1,..., sz-1.
	 */
	return distribution(generator);
}

void clear() {
	/* Clears the grid and empties points.
	 * Called after every successful find of a square to clear previous manipulations.
	 */
	for (int x = 0; x < sz; x++) {
		for (int y = 0; y < sz; y++) {
			grid[x][y] = false;					// Sets every value in 2D array grid to false
		}
	}
	pointpos = 0;								// Efficiently discards all elements of array points
}

void addPoint(Point t) {
	/* Adds Point t to array points and manipulates the grid accordingly.
	 * Called each time a point is added.
	 */
	int x = t.x;								// (int x, int y) stores coordinates of Point t
	int y = t.y;
	if (!grid[x][y]) {							// If the point on the grid is taken, skip
		grid[x][y] = true;						// Set (x, y) to true
		points[pointpos] = t;					// Appends (x, y) to array points
		pointpos++;								// Increments pointpos to next pointer
	}
}

bool pointEquals(Point p1, Point p2) {
	/* Checks whether p1 and p2 are equal.
	 * Called to check whether a point is equal to nullp.
	 */
	return ((p1.x == p2.x) && (p1.y == p2.y));
}

bool squareEquals(Square sq1, Square sq2) {
	/* Checks whether sq1 and sq2 are equal.
	 * Called to check whether a point is equal to nullsq.
	 */
	return (pointEquals(sq1.v1,sq2.v1) && pointEquals(sq1.v2,sq2.v2) &&
			pointEquals(sq1.v3,sq2.v3) && pointEquals(sq1.v4,sq2.v4));
}

Point randPoint() {
	/* Generates a random point (x, y).
	 * Coordinates are integers in 0, 1,..., sz-1.
	 * Note: Since density stays below ~1.7sz, choosing 1.7sz/sz^2 is unlikely
	 * and the algorithm does not noticeably slow down due to the while loop.
	 */
	while (true) {
		int x = randompos();				// Random number between 0 and sz-1, inclusive
		int y = randompos();
		if (!grid[x][y]) {					// If grid at (x,y) is empty, else retry
			Point returnpt;
			returnpt.x = x;
			returnpt.y = y;
			return returnpt;				// Return (x,y)
		}
	}
}

void gridPrint(Square result) {
	/* Prints grid out.
	 * Called every 1000th success if sz < 50.
	 */

	Point v1 = result.v1;								// v1, v2, v3, v4 are vertices of square result
	Point v2 = result.v2;
	Point v3 = result.v3;
	Point v4 = result.v4;

	for (int y = sz - 1; y > -1; y--) {
		for (int x = 0; x < sz; x++) {
			// For every (x, y); printing out flipped across diagonal to align with Cartesian grid

			if ((x == v1.x && y == v1.y) ||
					(x == v2.x && y == v2.y) ||
					(x == v3.x && y == v3.y) ||
					(x == v4.x && y == v4.y)) {			// If point is in square result
				cout << "# ";							// Print out a hash mark (square vertex)
				continue;								// Go to next point
			}
			cout << (grid[x][y] ? ". " : "  "); 		// Print period if normal point, space if no point
		}
		cout << endl;									// Skip to next line; end of row
	}
}

int distanceSq(Point p1, Point p2) {
	/* Finds distance squared between p1 and p2.
	 * Called after every success.
	 */

	int p1x = p1.x;										// Coordinates of p1, p2
	int p2x = p2.x;
	int p1y = p1.y;
	int p2y = p2.y;

	return ((p1x - p2x) * (p1x - p2x) +
			(p1y - p2y) * (p1y - p2y)); 				// Euclidean distance calculation (squared)
}

void gridPrint() {
	/* Prints grid without result (overloaded function).
	 * Unused.
	 */
	for (int y = sz - 1; y > -1; y--) {
		for (int x = 0; x < sz; x++) {
			// For every (x, y); printing out flipped across diagonal to align with Cartesian grid

			cout << (grid[x][y] ? ". " : "  "); 		// Print period if normal point, space if no point
		}
		cout << endl;									// Skip to next line; end of row
	}
}

bool inBounds(int n) {
	/* Sees if 0 <= n < sz (is n possibly the coordinate of a point)
	 * Called eight times per check.
	 */
	return (n >= 0 && n < sz);
}

Square check(Point t) {
	/* Sees if point t is the vertex of any new square.
	 * Called an average of 1.7sz times per success.
	 */
	int xtest1,xtest2;								// Vars symbolizing coordinates of 3rd and 4th vertices of a square
	int ytest1,ytest2;

	int x = t.x;									// (x, y) stores the coordinates of point t
	int y = t.y;

	for (int i = 0; i < pointpos; i++) {

		Point point = points[i];					// For every point in points
		if (pointEquals(t,point)) {					// If point is t, continue to next point
			continue;
		}

		int xt = point.x;							// Coordinates of point
		int yt = point.y;

		/* Case 1: square is on "left-hand side" of segment t -- point */

		xtest1 = xt + yt - y; xtest2 = x + yt - y;	// Coordinates of p1
		ytest1 = yt - xt + x; ytest2 = y - xt + x;	// Coordinates of p2

		if (inBounds(xtest1) && inBounds(ytest1) &&
				inBounds(xtest2) && inBounds(ytest2) &&
				grid[xtest1][ytest1] && grid[xtest2][ytest2]) {

			// Reached if a square satisfying Case 1 is found.
			// Constructs two points, p1 and p2, then constructs the Square data type and returns.

			Point p1,p2;

			p1.x = xtest1;
			p1.y = ytest1;
			p2.x = xtest2;
			p2.y = ytest2;

			Square returnsq;

			returnsq.v1 = t;
			returnsq.v2 = p1;
			returnsq.v3 = point;
			returnsq.v4 = p2;

			return returnsq;
		}

		/* Case 2: square is on "right-hand side" of segment t -- point */

		xtest1 = xt - yt + y; xtest2 = x - yt + y;	// Coordinates of p1
		ytest1 = yt + xt - x; ytest2 = y + xt - x;	// Coordinates of p2

		if (inBounds(xtest1) && inBounds(ytest1) &&
				inBounds(xtest2) && inBounds(ytest2) &&
				grid[xtest1][ytest1] && grid[xtest2][ytest2]) {

			// Reached if a square satisfying Case 1 is found.
			// Constructs two points, p1 and p2, then constructs the Square data type and returns.

			Point p1,p2;

			p1.x = xtest1;
			p1.y = ytest1;
			p2.x = xtest2;
			p2.y = ytest2;

			Square returnsq;

			returnsq.v1 = t;
			returnsq.v2 = p1;
			returnsq.v3 = point;
			returnsq.v4 = p2;

			return returnsq;
		}
	}

	return nullsq;									// Return the null square if no square was found.
}

int main() {

	cout.precision(10);								// Higher precision floats!

	clock_t start;									// Start a timing clock
	double duration;

	start = clock();

	unsigned long long int npoints = 0;				// Keeps running total of number of points necessary to make a square
	double sizes = 0;								// Keeps running total of side lengths of square

	nullp.x = -1;									// Initialize nullp
	nullp.y = -1;

	nullsq.v1 = nullp;								// Initialize nullsq
	nullsq.v2 = nullp;
	nullsq.v3 = nullp;
	nullsq.v4 = nullp;

	for (long int trial = 0; trial < trials; trial++) {

		clear();													// Clear the grid

		while (true) {
			Point pt = randPoint();									// Find an empty point
			addPoint(pt);											// Fill in that point
			Square result = check(pt);								// Does that point form the vertex of a square?

			if (result.v1.x == -1) {								// If result is nullsq, find another point
				continue;
			} else {
				npoints += pointpos;								// Add # of points to npoints
				sizes += sqrt(distanceSq(result.v2,result.v3));		// Add side length of square to sizes

				if (trial % 100 == 0) {									// If trial is a multiple of 10000

					cout << "Found square #" <<
							trial << "!" << endl;							// Print out trial #
					duration = (clock() - start) / (double) CLOCKS_PER_SEC;
					cout << "Seconds elapsed: " <<
							duration << endl;								// Print out seconds elapsed
					cout << "Square Calculations / Second: " <<
							trial/duration << endl;							// Print out square calculation rate
					cout << "Current PointN Average: " <<
							npoints/(double)trial << endl;					// Current average of number of points to make a square
					cout << "Current Square Average: " <<
							sizes/(double)trial << endl;						// Current average of side length of square
					cout << "Current PointN/Size Average: " <<
							npoints/((double)trial * sz) << endl;			// Current ratio between average of number of points and size

					if (sz < 10) {
						cout << "Grid: " << endl;
						gridPrint(result);									// Print the grid if the size is sufficiently small
					}

					cout << endl;											// Newline to separate success statements.

				}

				break;												// Go to next trial
			}
		}
	}

	cout << "====================================\n";
	duration = (clock() - start) / (double) CLOCKS_PER_SEC; // Total elapsed time
	cout << "Total Computational Time: " <<
			duration << endl;								// Seconds elapsed
	cout << "Square Calculations / Second: " <<
			trials/duration << endl;						// Square calculation rate
	cout << "PointN Average: " <<
			npoints/(double)trials << endl;					// Average of number of points to make a square
	cout << "Square Average: " <<
			sizes/(double)trials << endl;					// Average of side length of square
	cout << "PointN/Size Average: " <<
			npoints/((double)trials * sz) << endl;			// Ratio between average of number of points and size

	cout << endl;

	cout << "Completed " << trials << " trials on grids of size " << sz << "; Process complete!" << endl;
	return 0;
}
