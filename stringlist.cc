// stringlist.cc - Functions for working with lists of strings in plain C.
// See stringlist.h for documentation regarding the use of these functions.
// By K. Walsh <kwalsh@cs.holycross.edu>, Spring 2016

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stringlist.h"
#include "lexer.h"

// Note: Unlike most C code, this code has a lot of verbose error checking to
// help novice programers discover bugs in client code.

char **stringlist_empty() {
    // An empty list is an array containing one element, a NULL.
    char **list = (char **)malloc(1 * sizeof(char *));
    list[0] = NULL;
    return list;
}

void stringlist_print(char **list) {
    if (list == NULL) {
        printf("list is NULL\n");
        return;
    }
    int n = stringlist_len(list);
    printf("list (%d elements) = { ", n);
    for (int i = 0; list[i] != NULL; i++) {
        if (i != 0)
            printf(", ");
        printf("\"%s\"", list[i]);
    }
    printf(" }\n");
}

char *stringlist_to_string(char **list) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return strdup("");
    }
    return stringlist_join(list, "");
}

static void strjoin(char **s, const char *suffix) {
    char *t = (char *)malloc(strlen(*s) + strlen(suffix) + 1);
    strcpy(t, *s);
    strcat(t, suffix);
    *s = t;
}

char *stringlist_join(char **list, const char *sep) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return strdup("");
    }
    if (sep == NULL) {
        fprintf(stderr, "Error in %s: sep can't be NULL\n", __PRETTY_FUNCTION__);
        sep = "";
    }
    char *s = strdup("");
    for (int i = 0; list[i] != NULL; i++) {
        if (i != 0)
            strjoin(&s, sep);
        strjoin(&s, list[i]);
    }
    return s;
}

void stringlist_append(char ***list, const char *str) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: pointer to stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return;
    }
    if (*list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return;
    }
    if (str == NULL) {
        fprintf(stderr, "Error in %s: str can't be NULL\n", __PRETTY_FUNCTION__);
        return;
    }
    int n = stringlist_len(*list);
    *list = (char **)realloc(*list, (n + 2) * sizeof(char *));
    (*list)[n] = strdup(str);
    (*list)[n+1] = NULL;
}

int stringlist_len(char **list) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return 0;
    }
    int n = 0;
    while (list[n] != NULL)
        n++;
    return n;
}

int stringlist_find(char **list, const char *target) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return -1;
    }
    if (target == NULL) {
        fprintf(stderr, "Error in %s: target can't be NULL\n", __PRETTY_FUNCTION__);
        return -1;
    }
    for (int i = 0; list[i] != NULL; i++) {
        if (!strcmp(list[i], target)) {
            return i;
        }
    }
    return -1;
}

char **stringlist_dup(char **list) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    char **copy = stringlist_empty();
    for (int i = 0; list[i] != NULL; i++) {
        stringlist_append(&copy, list[i]);
    }
    return copy;
}

char **stringlist_copy(char **list, int start, int end) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    if (start < 0) {
        fprintf(stderr, "Error in %s: start position can't be negative\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    if (end > stringlist_len(list)) {
        fprintf(stderr, "Error in %s: end position can't be past the end of the list\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    if (start > end) {
        fprintf(stderr, "Error in %s: start positoin can't be after end position\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    char **list2 = stringlist_empty();
    for (int i = start; i < end; i++) {
        stringlist_append(&list2, list[i]);
    }
    return list2;
}

char *stringlist_pop(char ***list) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return strdup("");
    }
    int n = stringlist_len(*list);
    if (n <= 0) {
        fprintf(stderr, "Error in %s: stringlist can't be empty\n", __PRETTY_FUNCTION__);
        return strdup("");
    }
    char *removed = (*list)[n - 1];
    (*list)[n - 1] = NULL;
    return removed;
}

char **stringlist_popn(char ***list, int count) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    int n = stringlist_len(*list);
    if (count < 0) {
        fprintf(stderr, "Error in %s: count can't be negative\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    if (count > n) {
        fprintf(stderr, "Error in %s: count can't be larger than list length\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    char **list2 = (char **)malloc((count + 1) * sizeof(char *));
    for (int i = 0; i < count; i++) {
        list2[i] = (*list)[n - count + i];
        (*list)[n - count + i] = NULL;
    }
    list2[count] = NULL;
    return list2;
}

char **stringlist_split(char ***list, int pos) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: pointer to stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    if (*list == NULL) {
        fprintf(stderr, "Error in %s: stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    int n = stringlist_len(*list);
    if (pos < 0) {
        fprintf(stderr, "Error in %s: pos can't be negative\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    if (pos > n) {
        fprintf(stderr, "Error in %s: pos can't be larger than list length\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }
    int count = n - pos;
    char **list2 = (char **)malloc((count + 1) * sizeof(char *));
    for (int i = 0; i < count; i++) {
        list2[i] = (*list)[n - count + i];
        (*list)[n - count + i] = NULL;
    }
    list2[count] = NULL;
    return list2;
}

void stringlist_free(char ***list) {
    if (list == NULL) {
        fprintf(stderr, "Error in %s: pointer to stringlist can't be NULL\n", __PRETTY_FUNCTION__);
        return;
    }
    if (*list == NULL)
        return;
    for (int i = 0; (*list)[i] != NULL; i++) {
        free((*list)[i]);
    }
    free(*list);
    *list = NULL;
}

char **split_words(const char *str) {
    if (str == NULL) {
        fprintf(stderr, "Error in %s: str can't be NULL\n", __PRETTY_FUNCTION__);
        return stringlist_empty();
    }

    char **list = stringlist_empty();

    // Check if line was NULL or empty to start. 
    if (str[0] == '\0')
        return list;

    lexer x;
    lexer_init(&x, str);

    // Expecting zero or more words.
    lexer_next(&x);
    while (x.ttype == WORD) {
        stringlist_append(&list, x.tstr);
        lexer_next(&x);
    }

    if (x.errmsg) {
        fprintf(stderr, "Error in %s: %s\n", __PRETTY_FUNCTION__, x.errmsg);
        lexer_destroy(&x);
        stringlist_free(&list);
        return stringlist_empty();
    }

    lexer_destroy(&x);

    return list;
}

#ifdef UNITTEST

int main(void) {
    char **list = stringlist_empty();
    stringlist_print(list); // { }
    stringlist_append(&list, "Ada");
    stringlist_print(list); // { "Ada" }
    stringlist_append(&list, "Bob");
    stringlist_print(list); // { "Ada", "Bob" }
    stringlist_append(&list, "Cal");
    stringlist_print(list); // { "Ada", "Bob", "Cal" }


    char *name = stringlist_pop(&list);
    stringlist_print(list); // { "Ada", "Bob" }
    stringlist_append(&list, name);
    char **list2 = stringlist_popn(&list, 2);
    stringlist_print(list);  // { "Ada" }
    stringlist_print(list2); // { "Bob", "Cal" }
    stringlist_free(&list);
    stringlist_free(&list2);

    list = split_words("Ada Bob Cal");
    stringlist_print(list); // { "Ada", "Bob", "Cal" }
    stringlist_free(&list);

    list = split_words("'Ada Bob' Cal");
    stringlist_print(list); // { "Ada Bob", "Cal" }
    stringlist_free(&list);

    list = split_words("Ada 'Bob Cal'");
    stringlist_print(list); // { "Ada", "Bob Cal" }
    stringlist_free(&list);

    list = split_words("\"Ada\\n\\t \\\"Bob\\'\" Cal");
    stringlist_print(list); // { "Ada
                            //        "Bob'", "Cal" }
    stringlist_free(&list);

    list = split_words("Ada\\ Bob \\ Cal\\ ");
    stringlist_print(list); // { "Ada Bob",  " Cal " }
    stringlist_free(&list);

    list = split_words("Ada\\#Bob\\\"Cal\\t\\'");
    stringlist_print(list); // { "Ada#Bob"Cal    '" }
    stringlist_free(&list);

    list = split_words("Ada Bob Cal Foo Bar Baz");
    list2 = stringlist_dup(list);
    stringlist_print(list2); // { "Ada", "Bob", "Cal", "Foo", "Bar", "Baz" }
    stringlist_free(&list2);

    list2 = stringlist_copy(list, 0, 3);
    stringlist_print(list2); // { "Ada", "Bob", "Cal" }
    stringlist_free(&list2);

    list2 = stringlist_copy(list, 3, 6);
    stringlist_print(list2); // { "Foo", "Bar", "Baz" }
    stringlist_free(&list2);

    list2 = stringlist_copy(list, 2, 3);
    stringlist_print(list2); // { "Cal" }
    stringlist_free(&list2);

    list2 = stringlist_copy(list, 2, 2);
    stringlist_print(list2); // { }
    stringlist_free(&list2);

    printf("%d\n", stringlist_find(list, "Ada")); // 0
    printf("%d\n", stringlist_find(list, "Bob")); // 1
    printf("%d\n", stringlist_find(list, "Baz")); // 5
    printf("%d\n", stringlist_find(list, "Boo")); // -1
    stringlist_free(&list);

    list = split_words("Ada Bob Cal Foo Bar Baz");
    list2 = stringlist_split(&list, 2);
    stringlist_print(list);  // { "Ada", "Bob" }
    stringlist_print(list2); // { "Cal", "Foo", "Bar", "Baz" }
    stringlist_free(&list);
    stringlist_free(&list2);

    list = split_words("Ada Bob Cal Foo Bar Baz");
    list2 = stringlist_split(&list, 1);
    stringlist_print(list);  // { "Ada" }
    stringlist_print(list2); // { "Bob", "Cal", "Foo", "Bar", "Baz" }
    stringlist_free(&list);
    stringlist_free(&list2);

    list = split_words("Ada Bob Cal Foo Bar Baz");
    list2 = stringlist_split(&list, 0);
    stringlist_print(list);  // { }
    stringlist_print(list2); // { "Ada", "Bob", "Cal", "Foo", "Bar", "Baz" }
    stringlist_free(&list);
    stringlist_free(&list2);

    list = split_words("Ada Bob Cal Foo Bar Baz");
    list2 = stringlist_split(&list, 5);
    stringlist_print(list);  // { "Ada", "Bob", "Cal", "Foo", "Bar" }
    stringlist_print(list2); // { "Baz" }
    stringlist_free(&list);
    stringlist_free(&list2);

    list = split_words("Ada Bob Cal Foo Bar Baz");
    list2 = stringlist_split(&list, 6);
    stringlist_print(list);  // { "Ada", "Bob", "Cal", "Foo", "Bar", "Baz" }
    stringlist_print(list2); // { }
    stringlist_free(&list);
    stringlist_free(&list2);

    list = split_words("Ada Bob Cal");
    char *s = stringlist_to_string(list);
    printf("%s\n", s); // AdaBobCal
    free(s);
    s = stringlist_join(list, "--");
    printf("%s\n", s); // Ada--Bob--Cal
    free(s);

    // Error handling

    list2 = split_words("Ada Bob Cal");
    stringlist_free(&list); // no error
    stringlist_free(NULL); // error
    stringlist_print(list); // error
    stringlist_to_string(list); // error
    stringlist_append(NULL, "Foo"); // error
    stringlist_append(&list, "Foo"); // error
    stringlist_append(&list2, NULL); // error
    stringlist_len(list); // error
    stringlist_find(list, "Foo"); // error
    stringlist_find(list2, NULL); // error
    stringlist_dup(list); // error
    stringlist_copy(list, 0, 0); // error
    stringlist_split(NULL, 0); // error
    stringlist_split(&list, 0); // error
    stringlist_split(&list2, -1); // error

    return 0;
}
#endif // UNITTEST
