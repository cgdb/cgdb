#include "kui.h"
#include "error.h"
#include "sys_util.h"

/** This simply maintains a list of sequence/key pairs. */
struct kui_sequence_set {

};

/** Needs doco */
struct kuictx {
	/*
	int stdinfd;
	struct kui_sequence_set *kui_ss;
	struct input *ictx;
	*/
};

struct kuictx *kui_create(int stdinfd) {
	struct kuictx *kctx = (struct kuictx *)xmalloc(sizeof(struct kuictx));

	if ( (kctx->ictx = input_init ( stdinfd ) ) == NULL )
		return NULL;

	return kctx;
}

int kui_destroy ( struct kuictx *kctx ) {
	if ( !kctx )
		return -1;

	free (kctx);
	kctx = NULL;

	return 0;
}

struct kui_sequence_set *kui_ss_create ( void ) {
	struct kui_sequence_set *kui_ss = 
		(struct kui_sequence_set *)xmalloc(sizeof(struct kui_sequence_set));

	return kui_ss;
}

int kui_ss_destroy ( struct kui_sequence_set *kui_ss ) {
	if ( !kui_ss )
		return -1;

	free (kui_ss);
	kui_ss = NULL;

	return 0;
}

int kui_ss_register_sequence ( 
		struct kui_sequence_set *kui_ss,
		const int sequence[], 
		const int size,
	    kui_sequence_key *key ) {

	/* Not implemented */
	return -1;
}

int kui_ss_deregister_sequence (
		struct kui_sequence_set *kui_ss,
		kui_sequence_key *key ) {
	/* Not implemented */
	return -1;
}

struct kui_sequence_set *kui_get_sequence_set ( struct kuictx *kctx ) {
	if ( !kctx )
		return NULL;

	return kctx->kui_ss;
}

int kui_set_sequence_set ( 
		struct kuictx *kctx, 
		struct kui_sequence_set *kui_ss ) {
	if ( !kctx )
		return -1;

	kctx->kui_ss = kui_ss;
	return 0;
}

int kui_getkey ( 
		struct kuictx *kctx,
	    kui_sequence_key *key,
	    int *input_key	) {

	/* Not implemented */
	return -1;
}
