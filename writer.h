#include "parser.h"

#include "text.h"
#include "html.h"
#include "latex.h"
#include "memoir.h"
#include "beamer.h"
#include "man.h"
#include "opml.h"
#include "odf.h"
#include "critic.h"

char * export_node_tree(node *list, int format, int extensions);

void extract_references(node *list, scratch_pad *scratch);
link_data * extract_link_data(char *label, scratch_pad *scratch);

void pad(GString *out, int num, scratch_pad *scratch);

int note_number_for_label(char *text, scratch_pad *scratch);
node * node_matching_label(char *label, node *n);
int count_node_from_end(node *n);
int cite_count_node_from_end(node *n);
node * node_for_count(node *n, int count);
void move_note_to_used(node *list, scratch_pad *scratch);
node * node_for_attribute(char *querystring, node *list);

char * dimension_for_attribute(char *querystring, node *list);
