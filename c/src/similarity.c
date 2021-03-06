#include "similarity.h"
#include "search.h"
#include "array.h"
#include "helper.h"
#include "symbol.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "internal.h"

/****************************************************************************
 *
 * Term
 *
 ****************************************************************************/

Term *term_new(Symbol field, const char *text)
{
    Term *t = ALLOC(Term);
    t->field = field;
    t->text = estrdup(text);
    return t;
}

void term_destroy(Term *self)
{
    free(self->text);
    free(self);
}

int term_eq(const void *t1, const void *t2)
{
    return (strcmp(((Term *)t1)->text, ((Term *)t2)->text)) == 0 &&
        (((Term *)t1)->field == ((Term *)t2)->field);
}

unsigned long term_hash(const void *t)
{
    return str_hash(((Term *)t)->text) * sym_hash(((Term *)t)->field);
}

/****************************************************************************
 *
 * Similarity
 *
 ****************************************************************************/

static float simdef_length_norm(Similarity *s, Symbol field, int num_terms)
{
    (void)s;
    (void)field;
    return (float)(1.0 / sqrt(num_terms));
}

static float simdef_query_norm(struct Similarity *s, float sum_of_squared_weights)
{
    (void)s;
    return (float)(1.0 / sqrt(sum_of_squared_weights));
}

static float simdef_tf(struct Similarity *s, float freq)
{
    (void)s;
    return (float)sqrt(freq);
}

static float simdef_sloppy_freq(struct Similarity *s, int distance)
{
    (void)s;
    return (float)(1.0 / (double)(distance + 1));
}

static float simdef_idf_term(struct Similarity *s, Symbol field, char *term,
                      Searcher *searcher)
{
    return s->idf(s, searcher->doc_freq(searcher, field, term),
                  searcher->max_doc(searcher));
}

static float simdef_idf_phrase(struct Similarity *s, Symbol field,
                        PhrasePosition *positions,
                        int pp_cnt, Searcher *searcher)
{
    float idf = 0.0;
    int i, j;
    for (i = 0; i < pp_cnt; i++) {
        char **terms = positions[i].terms;
        for (j = ary_size(terms) - 1; j >= 0; j--) {
            idf += sim_idf_term(s, field, terms[j], searcher);
        }
    }
    return idf;
}

static float simdef_idf(struct Similarity *s, int doc_freq, int num_docs)
{
    (void)s;
    return (float)(log((float)num_docs/(float)(doc_freq+1)) + 1.0);
}

static float simdef_coord(struct Similarity *s, int overlap, int max_overlap)
{
    (void)s;
    return (float)((double)overlap / (double)max_overlap);
}

static float simdef_decode_norm(struct Similarity *s, uchar b)
{
    return s->norm_table[b];
}

static uchar simdef_encode_norm(struct Similarity *s, float f)
{
    (void)s;
    return float2byte(f);
}

static void simdef_destroy(Similarity *s)
{
    (void)s;
    /* nothing to do here */
}

static Similarity default_similarity = {
    NULL,
    {0},
    &simdef_length_norm,
    &simdef_query_norm,
    &simdef_tf,
    &simdef_sloppy_freq,
    &simdef_idf_term,
    &simdef_idf_phrase,
    &simdef_idf,
    &simdef_coord,
    &simdef_decode_norm,
    &simdef_encode_norm,
    &simdef_destroy
};

Similarity *sim_create_default()
{
    int i;
    if (!default_similarity.data) {
        for (i = 0; i < 256; i++) {
            default_similarity.norm_table[i] = byte2float((unsigned char)i);
        }

        default_similarity.data = &default_similarity;
    }
    return &default_similarity;
}
