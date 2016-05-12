#ifndef STRINGLIST_H
#define STRINGLIST_H

// stringlist.h - Functions for working with lists of strings in plain C.
// By K. Walsh <kwalsh@cs.holycross.edu>, Spring 2016

// A stringlist is an array of pointers to strings. For example:
//   char **list;
// The last element of the array must always NULL, and none of the other
// elements can be NULL. This is, not coincidentally, exactly the style of data
// structure used for (argv, argc) arguments for main() and for the execv*()
// family of system calls.
//
// An example stringlist might look like:
//   { "Hello", "World", NULL }
// This shows two strings (length = 2), but note that the underlying array
// actually is length 3 because it also holds a NULL in the last position.
//
// You can call stringlist_len() to find the number of strings in the list. In
// the above example, the length would be 2, since again, the length doesn't
// count the NULL at the end.
//
// You can iterate over the elements like this:
//   for (int i = 0; list[i] != NULL; i++)
//     printf("Element %d is %s\n", i, list[i]);
//
// Or, you can get the length and iterate like this:
//   int n = stringlist_len(list);
//   for (int i = 0; i < n; i++)
//     printf("Element %d is %s\n", i, list[i]);
//
// You can check if a list is empty by checking if the first element is NULL.
//    if (list[0] == NULL) { printf("List is empty\n"); }
// Or, you can check if the length is zero:
//    if (stringlist_len(list) == 0) { printf("List is empty\n"); }


// Create and return a new stringlist containing no strings.
char **stringlist_empty();

// Free the memory associated with a stringlist. This calls free() for each
// individual string in the list, and then calls free() on the underlying list
// array itself. It also sets the list to NULL, just for good measure. A pointer
// to the stringlist is passed in, because this function modifies the list
// parameter. 
void stringlist_free(char ***list);

// Split a string into "words" and returns a stringlist containing the results.
// For example:
//   char **words = split_words("Ada is a nice name");
// After this code executes, words will be an array like this:
//     { "Ada", "is", "a", "nice", "name", NULL }.
// Roughly, this function treats anything between whitespace as a "word" and it
// discards all the whitespace between words. There are three exceptions:
//
//   * A single-quoted or double-quoted string is considered to be a single
//     word, even if it has whitespace inside it. So 'Hello World' counts as a
//     single "word". Same for "Hello World". A single-quoted string can contain
//     double-quote characters, and vice versa. Backslash-escapes (like "\n",
//     "\'" and "\t") are also recognized within quoted strings.
//
//   * If a space or tab is preceeded by a backslash '\', then it is not
//     considered whitespace, but instead counts as part of a word. For example,
//     Ada\ Bob is a single "word".
//
//   * If a pound '#' character appears outside a quoted string, then that
//     character and anything after it is completely ignored.
//
// This function never returns NULL, though it sometimes prints an error message
// and returns an empty list.
//
// When you are finished with the resulting stringlist, call stringlist_free()
// to free the memory associated with the list. Alternatively, you can call
// free() on each element of the list, then call free() on the entire list. 
char **split_words(const char *str);

// Print a stringlist to standard output. This is mostly for debugging.
void stringlist_print(char **list);

// Convert a stringlist to a single string and return that string. For example,
//   stringlist_to_string({"A", "B", "C", NULL}) --> "ABC"
// This is the same as stringlist_join(list, "").
char *stringlist_to_string(char **list);

// Join the elements a stringlist together, separated by sep, and return the
// resulting string. For example,
//   stringlist_to_string({"A", "B", "C", NULL}, ", ") --> "A, B, C"
char *stringlist_join(char **list, const char *sep);

// Add a new string str to the end of a stringlist. This new string will replace
// the NULL that was previously at the end of the underlying array and put a new
// NULL at the new end. A copy of str is made. A pointer to the stringlist is
// passed in, because this function modifies the list parameter. For example, to
// make a stringlist containing three strings:
//   char **list = stringlist_empty();
//   stringlist_append(&list, "Alice");
//   stringlist_append(&list, "Bob");
//   stringlist_append(&list, "Claire");
void stringlist_append(char ***list, const char *str);

// Count the number of strings in a stringlist.
int stringlist_len(char **list);

// Search a stringlist for a target word and return the position of the word if
// it was found within the list. Otherwise, if the word was not found in the
// list, return -1.
int stringlist_find(char **list, const char *target);

// Create and return a copy of a stringlist. A copy of element is made.
char **stringlist_dup(char **list);

// Create and return a copy of a portion of a stringlist. A copy of each element
// is made, starting at position start, up to but not including position end.
// The original list is not modified. 
char **stringlist_copy(char **list, int start, int end);

// Remove and return the last element of a stringlist. When you are done with
// the returned string, you should call free() to free it. A pointer to the
// stringlist is passed in, because this function modifies the list parameter.
// For example:
//   char *last = stringlist_pop(&list);
//   printf("The last element was %s\n", last);
//   free(last);
char *stringlist_pop(char ***list);

// Remove and return the last count elements of a stringlist. When you are done
// with the returned stringlist, you should call stringlist_free() to free it. A
// pointer to the stringlist is passed in, because this function modifies the
// list parameter. For example:
//   char **last = stringlist_popn(&list, 3);
//   char *s = stringlist_to_string(last);
//   printf("The last three elements were %s\n", s);
//   stringlist_free(last);
char **stringlist_popn(char ***list, int count);

// Split a stringlist into two parts. The elements up to (but not including) pos
// are left in the original list, and the elements starting at pos through the
// end are moved into a new list, which is returned. When you are done with the
// returned stringlist, you should call stringlist_free() to free it. A pointer
// to the stringlist is passed in, because this function modifies the list
// parameter. This is the same as calling stringlist_popn(list, n-pos), where n
// is the length of the list.
char **stringlist_split(char ***list, int pos);

#endif // STRINGLIST_H
