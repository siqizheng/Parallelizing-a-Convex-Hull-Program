// --------------------- convex_mp_0411.cpp -----------------------------------------
//
// Siqi Zheng, CSS 503
// Created:         Apr. 7, 2020
// Last Modified:   Apr. 12, 2020
// --------------------------------------------------------------
// Purpose: parallelize a convex hull program with multiple processes.
//algorithm used here is a combination of divide - and -conquer and Graham¡¯s scan.
// verify if the performance is improved or not with multi processes
// --------------------------------------------------------------
// Assumptions: All input data is assumed to be correct and valid. 
// ---------------------------------------------------------------------------

#include <iostream>
#include <deque>
#include <sys/time.h> 
#include <math.h>      
#include <sys/types.h> // fork, wait
#include <sys/wait.h> // wait
#include <unistd.h> // fork
#include <stdlib.h>// for exit
#include <stdio.h>

#define DEBUG "yes, its in a debugging mode"

using namespace std;

/*
 * defines each point in a convex hull.
 */
class Point 
{
public:
   double x, y;          // x and y coordinates
   int id;               // 0-indexed point identifier

   Point(double initX, double initY, int id) 
   {
      this->x = initX;
      this->y = initY;
      this->id = id;
   }

   Point()
   {
      x = 0.0;
      y = 0.0;
      id = -1;
   }

   void print() 
   {
      printf( "point[%d].x = %f .y = %f\n", id, x, y );
   }
};

/*
 * Initialize nPoints, each with random values for (x, y) and an increasing id.
 * The deque holds the points
 */
void init(deque<Point> &q, int nPoints) 
{
   string response;
   cout << "do you want to display initial data? ";
   cin >> response;
   for (int i = 0; i < nPoints; i++) 
   {
      q.push_back(*(new Point(rand() % 10000, rand() % 10000, i)));
      if (response == "y")
      {
         q.back( ).print( );
      }
   }
}

void divide(deque<Point> &q, deque<Point> &s1, deque<Point> &s2) 
{
   for (int i = q.size() / 2; i > 0; i--) 
   {
      s1.push_back(q.front());
      q.pop_front();
   }
   while (!q.empty()) 
   {
      s2.push_back(q.front());
      q.pop_front();
   }
}

/**
 * Computes the polar angle and distance between the origin o and the
 * the other point p.
 *
 * @param o          the orign point
 * @param p          the other point
 * @param distance   the distance between o and p (to be returned)
 */
double polar_angle(Point &o, Point &p, double &distance ) 
{
   double cos;

   if (o.x == p.x && o.y == p.y) 
   {
      distance = 0;
      cos = 1;
   }
   else 
   {
      distance = sqrt(pow(p.x - o.x, 2.0) + pow(p.y - o.y, 2.0));
      cos = (p.x - o.x) / distance;
   }
   return(-cos);
}

/**
 * Merges two deques s1 and s2 into q as removing those points not
 * in polar angles.
 * @param q     a deque to merge s1 and s2
 * @param s1    a deque from the left child
 * @param s2    a deque from the right child
 */
void merge(deque<Point> &q, deque<Point> &s1, deque<Point> &s2) 
{
  // Error checking
   if (!q.empty()) 
   {
      cerr << "merge: q is not empty." << endl;
      exit( -1 );
   }

   // decide deque1 and deque2
   // deque( s1 or s2 ) with the lowest leftmost point becomes deque1.
   deque<Point> &deque1 =
    ( s2.empty( ) ) ? s1 :
    ( s1.empty( ) ) ? s2 :
    ( s1.front( ).y == s2.front( ).y ) ?
    ( ( s1.front( ).x == s2.front( ).y ) ? s1 ://x?
      ( s1.front( ).x < s2.front( ).x ) ?  s1 : s2 ) :
    ( s1.front( ).y < s2.front( ).y ) ? s1 : s2;

  deque<Point> &deque2 = (&deque1 == &s1) ? s2 : s1;
  Point origin = deque1.front();

  // if deque1 and deque2 have the same first point, discard it from deque2.
   if (!deque2.empty() &&                // at least deque1 must have some
        (deque1.front().x == deque2.front().x) &&
        (deque1.front().y == deque2.front().y))
   {
      deque2.pop_front( );
   }

   // discard the wedge of deque_2 which will be internal of S1 U S2
   deque<Point> new_deque2;
   bool external_wedge = false;

   // polar angle and distance of deque2's 1st point
   double polar1 = 0.0;
   double distance1 = 0.0;

   // polar angle and distance of deque2's 2nd point
   double polar2 = 0.0;
   double distance2 = 0.0;

   Point deque2_original_top; // deque2's original top point
   if (!deque2.empty())
   {
      deque2_original_top = deque2.front();
   }

   while (!deque2.empty()) 
   {
      Point cur = deque2.front( ); 
      deque2.pop_front();   // pick up deque2's first.
      if (deque2.empty())    // deque2 has only 1 point
      {                  
         if (cur.id == deque2_original_top.id)  // deque2 originally had only 1
         {
	    new_deque2.push_back(cur);
	    break;
         }                      // this is eventually the last 1 in deque2
         polar2 = polar_angle(origin, deque2_original_top, distance2);
      }
      else                            // check the next in deque2
      {
         polar2 = polar_angle(origin, deque2.front(), distance2);
      }

      polar1 = polar_angle(origin, cur, distance1); // check the 1st in deque2

      // now compare 1st and 2nd point of deque2 regarding polar angle & distance
      if (polar1 < polar2) 
      {
         new_deque2.push_back(cur); // to make a convex
         external_wedge = true;
         if (deque2.empty() && (new_deque2.front().id != deque2_original_top.id))
         {
	    // this was the very last 1 in deque2
	    new_deque2.push_back( deque2_original_top );
         }
      }
      else if (polar1 == polar2) 
      {
          if ( distance1 > distance2 )
          {
	     new_deque2.push_back( cur ); // to make a convex
          }
      }
      else if (external_wedge == true) 
      { 
         // previous point was convex
         new_deque2.push_back( cur );            // this also makes a convex
         external_wedge = false;
      }
      // else { discard cur }
   }

   // now merge deque1 and new_deque2
   while ((! deque1.empty()) || (! new_deque2.empty())) 
   {
      if ((! deque1.empty()) && new_deque2.empty()) 
      {
         // only deque1 remains
         q.push_back(deque1.front()); 
         deque1.pop_front();
         continue;
      }
      if (deque1.empty() && !new_deque2.empty()) 
      {
         // only deque2 remains
         q.push_back(new_deque2.front()); 
         new_deque2.pop_front();
         continue;
      }
      if ((! deque1.empty()) && (! new_deque2.empty())) 
      {
         // both deque1 and qeue2 have some.
         Point cur1 = deque1.front();
         Point cur2 = new_deque2.front();
         polar1 = polar_angle(origin, cur1, distance1);
         polar2 = polar_angle(origin, cur2, distance2);

         if (polar1 == polar2) 
         {
	    // both points have the same angle, then choose only the farther one.
	    if (distance1 > distance2)
            {
	       q.push_back(cur1);
            }
	    else
            {
	       q.push_back(cur2);
            }
	    deque1.pop_front();     // the one not chosen is simply discarded.
	    new_deque2.pop_front();
         }
         else if (polar1 < polar2) 
         {
	    // deque1 has a point to make a convex
	    q.push_back(cur1);
	    deque1.pop_front();
         }
         else 
         { 
            // if ( polar1 > polar2 )
	    // deque2 has a point to make a convex
	    q.push_back(cur2);
	    new_deque2.pop_front();
         }
      }
   }
}

/*
 * Checks if B is hit by line that revolves left on A to B
 *
 * @param a   the central point of a left-revovling line AC
 * @param b   the point to check
 * @param c   the destination to revolve line AC
 * @return    true if B is it by line AC that revolves left on A to B
 */
bool leftturn(Point &a, Point &b, Point &c) 
{
   double slope_ac;         // slope of line ac
   double disp;             // the intersection of line ac and x-axis

   if (c.x == a.x)          // slope_ac = inf
   {  
      if (c.y > a.y)         // slope_ac = +inf
      {
          if (b.x > a.x)       // b is right off ac that goes up
          {
             return true;       // left turn to b
          }
      } 
      else                // slope_ac = -inf
      {
         if (b.x < a.x)       // b is left off ac that goes down
         {
            return true;       // left turn to b
         }
      }
   } 
   else if (c.y == a.y)  // slope_ac = 0
   {
      if (c.x > a.x)        // slope_ac goes right straightly
      {
         if (b.y < a.y)       // b is below ac that goes right
         {
            return true;       // left turn to b
         }
      } 
      else                // slope_ac goes left striaghtly
      {
         if (b.y > a.y)       // b is above ac that goes left
         {
            return true;       // left turn to b
         }
      }
   } 
   else if ((slope_ac = (c.y - a.y)/(c.x - a.x)) > 0) 
   {                        // slope_ac > 0
      disp = a.y - slope_ac * a.x;
      if (c.y > a.y)       // slope_ac goes up right
      {
         if (b.y < slope_ac * b.x + disp)
         {
            return true;       // b is below ac
         }
      } 
      else                //  slope_ac goes down left
      {
         if (b.y > slope_ac * b.x + disp)
         {
            return true;       // b is above ac
         }
      }
   } 
   else                 // slope_ac < 0
   {
      disp = a.y - slope_ac * a.x;
      if (c.y > a.y)        // slope_ac goes up left
      {
          if (b.y > slope_ac * b.x + disp)
          {
             return true;       // b is above ac
          }
      } 
      else                // slope_ac goes down right
      {
         if (b.y < slope_ac * b.x + disp)
         {
            return true;       // b is below ac
         }
      }
   }
   return false;
}

/*
 * Performs the Graham algorithm that scan all points from the bottom as
 * revolving a line leftward and eliminating non-convex points.
 *
 * @param q   a list of Points to check if they are convex points.
 */
void graham(deque<Point> &q) 
{
   deque<Point> hull;
   Point last2, last1, next;

   // choose the first three points as convex-hull points.
   for (int i = 0; i < 3 && (! q.empty()); i++ ) 
   {
      last2 = last1;
      last1 = q.front();
      hull.push_back(last1); 
      q.pop_front();
   }

   while(!q.empty()) 
   {
      next = q.front(); 
      q.pop_front();
      if (leftturn(last2, last1, next) == false)
      {
         hull.pop_back();  // remove last1 that is no longer a convex point
      }
      last2 = hull.back();
      last1 = next;
      hull.push_back(next);
   }

   q = hull;
}

/*
 * Sends a deque to a destination process through a pipe fd.
 * @param fd the file descriptor of a pipe
 * @param q a deque to send
 */
void send(int fd, deque<Point> &q) 
{
   int size = q.size();
   double x[size];
   double y[size];
   int id[size];

   // Serialize all deque items to x, y and id arrays
   for (int i = 0; i < size; i++) 
   {
      Point p = q.front();
      x[i] = p.x;
      y[i] = p.y;
      id[i] = p.id;
      q.pop_front();
   }

   // send all data through a pipe
   write(fd, &size, sizeof(int));
   write(fd, x, sizeof(double) * size);
   write(fd, y, sizeof(double) * size);
   write(fd, id, sizeof(int) * size);
}

 /*
  * Recieves a deque from a source process through a pipe fd.
  * @param fd the file descriptor of a pipe
  * @param q a deque to recieve
  */
void recv(int fd, deque<Point> &q) 
{
   // receive all data through a pipe
   int size = 0;
   read(fd, &size, sizeof(int));

   double x[size];
   double y[size];
   int id [size];

   read(fd, x, sizeof(double ) * size);
   read(fd, y, sizeof(double ) * size);
   read(fd, id, sizeof(int) * size);

   // de-serialized x, y, and id arrays to all deque items
   for (int i = 0; i < size; i++) 
   {
      Point *p = new Point(x[i], y[i], id[i]);
      q.push_back(*p);
   }
}


/* 
 *  Divides a list of all points by two until each subset includes at
 *  least two, and thereafter merges those that make a convex.
 *  leaves is used for the multi-process version (Student to utilize)
 */
 void divide_and_conquer(deque<Point>& q, int leaves = 0)
{

    deque<Point> s1, s2;
    divide(q, s1, s2);    // divide in two sub-queues. 

    if (leaves > 1) 
   {   
       //declare file descriptor
        int pipeFD[2];
       //create a pipe
        if (pipe(pipeFD) < 0)
        {
            perror("Error in creating pipe");
            exit(EXIT_FAILURE);
        }
		//  fork a child process and let it work on the right subset, 
        //whereas have the original parent take care of the left subset.
        int returnPid = fork();
		if(returnPid < 0)//error in fork
		{
			perror("Error during fork");
            exit(EXIT_FAILURE);
        }
        else if (returnPid > 0) //parent
        {
            if (s1.size() > 1)
            {
                divide_and_conquer(s1, leaves/2);  // work on the left queue
            }
            wait(NULL); //wait until children process completed
    
            s2.clear(); //clear up the original left over data in the s2
            
           //parent receive sub convex hull from child through pipe
            close(pipeFD[1]);
            recv(pipeFD[0], s2);
        }
        else//child
        {
            if (s2.size() > 1)
            {
                divide_and_conquer(s2, leaves - leaves/2);  // work on the right queue
            }
      
            //send child's sub convex hull back to pipe
            close(pipeFD[0]);
            send(pipeFD[1], s2);
        
            exit(EXIT_SUCCESS);//when child exit, parent will stop waiting and execute codes below wait(NULL)
        }
    }
    else
    {
        if (s1.size() > 1)
        {
            divide_and_conquer(s1);  // work on the left queue
        }
        if (s2.size() > 1)
        {
            divide_and_conquer(s2);  // work on the right queue
        }
    }

    merge(q, s1, s2);
    graham(q);                 // apply the graham algorithm
}


/**
 * This is the main: a graham-based convex hull program
 */
int main(int argc, char *argv[]) 
{
   // argument verification
   if ((argc < 2 ) || (argc > 3))
   {
      cout << "Usage: " << argv[0] << " #Points #Leaves" << endl;
      exit( -1 );
   }

   // Create argv[1] random points and put them deque
   deque<Point> points;
   init(points, atoi(argv[1]));

   // timed convex hull operation
   struct timeval start;
   gettimeofday(&start, NULL);
   if(argc == 3)
   {
      divide_and_conquer(points, atoi(argv[2]));
   }
   else 
   {
      divide_and_conquer(points);
   }
   struct timeval end;
   gettimeofday(&end, NULL);

   cout << "elapsed time = " <<
    (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_usec - start.tv_usec) << endl;

   // result output
   string response;
   cout << "do you want to display result data? ";
   cin >> response;
   if ( response == "y" )
   {
      while (!points.empty()) 
      {
         (points.front()).print();
         points.pop_front( );
      }
   }
   return 0;
}

