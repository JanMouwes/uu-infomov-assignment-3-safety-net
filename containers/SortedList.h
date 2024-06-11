/**************************  SortedList.cpp   *********************************
* Author:        Agner Fog
* Date created:  2008-06-12
* Last modified: 2008-06-12
* Description:
* Template class for a sorted and searchable list
*
* The latest version of this file is available at:
* www.agner.org/optimize/cppexamples.zip
* (c) 2008 GNU General Public License www.gnu.org/copyleft/gpl.html
*******************************************************************************
*
* SortedList defines a list of items that is kept sorted at all times so that
* elements can be found fast by binary search.
* 
* The maximum size is defined at compile time. The elements in the array can 
* be of any type that do not require a constructor or destructor.
*
* If OBJTYPE is not a simple type then you must define the operator < for 
* operands of type const OBJTYPE &. This operator defines the sort criterion
* for sorting the records. You need not define the operator ==. Two records 
* a and b of type OBJTYPE are assumed to be equal if !(a < b || b < a).
*
* A sorted list is efficient for small lists because of its simplicity.
* A sorted list is not the optimal solution for large lists. In fact, it 
* can become very inefficient if the list is large and objects are added in 
* random order because, on average, half of the existing objects have to be 
* moved every time a new object is added. A binary tree or hash map is more
* efficient for large lists.
*
* If objects are big then it may be faster to store pointers or array indices
* to the objects in a sorted list than storing the objects themselves.
*
******************************************************************************/

#include <string.h>                    // For strcmp
#include <stdio.h>                     // Needed for example only


namespace Templ8
{
   // Template for sorted list
   template <typename OBJTYPE, unsigned int MAXSIZE>
   class SortedList {
   protected:
      unsigned int num;                   // Number of objects in list
      OBJTYPE list[MAXSIZE];              // Storage buffer
   public:
      // Constructor:
      SortedList() {
         num = 0;                         // Initialize
      }

      // Operator [] returns the entry with index i
      OBJTYPE & operator [] (unsigned int i) {
         if (i >= num) {
            // Index out of range. The next line provokes an error.
            // You may insert any other error reporting here:
            return *(OBJTYPE*)0;          // Return null reference
         }
         // No error
         return list[i];                  // Return reference
      }

      // This function gets the number of records in the list:
      unsigned int NumRecords() {
         return num;
      }

      // This function returns an index to the first entry 
      // bigger than or equal to x. If no entry is bigger than
      // or equal to x then the return value is num, which is
      // not a valid index.
      unsigned int Search(OBJTYPE const & x) {
         unsigned int a = 0;              // Start of search interval
         unsigned int b = num;            // End of search interval + 1
         unsigned int c = 0;              // Middle of search interval
         // Binary search loop:
         while (a < b) {
            c = (a + b) / 2;
            if (list[c] < x) {
               a = c + 1;
            }
            else {
               b = c;
            }
         }
         return a;                        // Result of search
      }

      // This function adds a new record to the list. 
      // Returns true if success, false if the list is full:
      bool Put(OBJTYPE const & x) {
         if (num >= MAXSIZE) {
            return false;                 // List full
         }
         unsigned int a = Search(x);      // Find correct storage place
         // Move all records from a and above up one place to make 
         // space for the new record:
         if (num > a) {
            memmove(list+a+1, list+a, (num-a) * sizeof(OBJTYPE));
         }
         // Copy record into the right place in list:
         list[a] = x;
         num++;                           // Increment count
         return true;                     // Success
      }

      // This function does the same as Put, except that it does not
      // put x into the list if an existing record is equal to x. This
      // prevents duplicates. Returns false if the list is full:
      bool PutUnique(OBJTYPE const & x) {
         if (num >= MAXSIZE) {
            return false;                 // List full
         }
         unsigned int a = Search(x);      // Find correct storage place
         if (a < num && !(x < list[a])) {
            // Equal object found
            return true;                  // Return without adding x
         }
         // Make space for the new record:
         if (num > a) {
            memmove(list+a+1, list+a, (num-a) * sizeof(OBJTYPE));
         }
         // Copy record into the right place in list:
         list[a] = x;
         num++;                           // Increment count
         return true;                     // Success
      }

      // This function removes a record with index i from the list:
      void Remove(unsigned int i) {
         if (i >= num) return;            // No record with index i
         if (num > i + 1) {            
            // Move remaining records down
            memmove(list+i, list+i+1, (num-i-1) * sizeof(OBJTYPE));
         }
         num--;                           // Count down num
      }

      // This function tells if a record equal to x is in the list.
      // The parameter Index will receive the index if found.
      bool Exists(OBJTYPE const & x, unsigned int & Index) {
         // Two records a and b are assumed to be equal if
         // !(a < b || b < a)
         unsigned int i = Search(x);
         if (i >= num || x < list[i]) {
            return false;                 // Not found
         }
         Index = i;
         return true;
      }
   };
}