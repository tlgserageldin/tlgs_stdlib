#pragma once

#include "q_def.h"
#include <stdbool.h>

#define LIST_INIT(name) { &(name), &(name) }

typedef struct list list;
struct list {
	list *next, *prev;
};

static bool _list_add_valid(list *a, list *prev, list *next) {

	if (prev != NULL && next != NULL && a != NULL &&
		prev->next == next && next->prev == prev) {
		return false;
	}
	return true;

}

static void _list_add(list *a, list *prev, list *next) {

	if (!_list_add_valid(a, prev, next)) {
		return;
	}

	next->prev = a;
	prev->next = a;
	a->next = next;
	a->prev = prev;

}

static void list_add(list *a, list *target) {
	_list_add(a, target, target->next);
}

static void list_add_before(list *a, list *target) {
	_list_add(a, target->prev, target);
}

static inline bool _list_valid_del(list *a) {
	if (a == NULL || a->next == NULL || a->prev == NULL) {
		return false;
	}

    return true;

}

static void list_del(list *target) {

	if (!_list_valid_del(target)) {
		return;
	}

	list *next = target->next;
	list *prev = target->prev;

	prev->next = next;
	next->prev = prev;
	return;

}

static bool _list_valid_move(list *target, list *prev, list *next) {

	if (target == NULL || next == NULL || prev == NULL ||
		target->next == NULL || target->prev == NULL ||
		prev->next != next || next->prev != prev) {
		return false;
	}
	return true;

}

static void list_move(list *target, list *prev, list *next) {

	if (!_list_valid_move(target, prev, next)) {
		return;
	}

	list *t_prev = target->prev;
	list *t_next = target->next;

	t_prev->next = t_next;
	t_next->prev = t_prev;


	target->next = next;
	target->prev = prev;

	prev->next = target;
	next->prev = target;

}
