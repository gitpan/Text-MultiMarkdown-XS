/*

	writer.c -- General routines for converting parse structure to various 
		output formats.

	(c) 2013 Fletcher T. Penney (http://fletcherpenney.net/).

	Derived from peg-multimarkdown, which was forked from peg-markdown,
	which is (c) 2008 John MacFarlane (jgm at berkeley dot edu), and 
	licensed under GNU GPL or MIT.

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License or the MIT
	license.  See LICENSE for details.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

*/

#include "writer.h"

/* export_node_tree -- given a tree, export as specified format */
char * export_node_tree(node *list, int format, int extensions) {
	char *output;
	GString *out = g_string_new("");
	scratch_pad *scratch = mk_scratch_pad(extensions);

#ifdef DEBUG_ON
	fprintf(stderr, "export_node_tree\n");
#endif

#ifdef DEBUG_ON
	fprintf(stderr, "extract_references\n");
#endif
	/* Parse for link, images, etc reference definitions */
	if ((format != OPML_FORMAT) &&
		(format != CRITIC_ACCEPT_FORMAT) &&
		(format != CRITIC_REJECT_FORMAT) &&
		(format != CRITIC_HTML_HIGHLIGHT_FORMAT))
			extract_references(list, scratch);
	
	/* Change our desired format based on metadata */
	if (format == LATEX_FORMAT)
		format = find_latex_mode(format, list);
	
	switch (format) {
		case TEXT_FORMAT:
			print_text_node_tree(out, list, scratch);
			break;
		case HTML_FORMAT:
			if (scratch->extensions & EXT_COMPLETE) {
			    g_string_append_printf(out,
				"<!DOCTYPE html>\n<html>\n<head>\n\t<meta charset=\"utf-8\"/>\n");
			}
#ifdef DEBUG_ON
	fprintf(stderr, "print_html output\n");
#endif
			print_html_node_tree(out, list, scratch);
#ifdef DEBUG_ON
	fprintf(stderr, "print html endnotes\n");
#endif
			print_html_endnotes(out, scratch);
#ifdef DEBUG_ON
	fprintf(stderr, "finished printing html endnotes\n");
#endif
			if (scratch->extensions & EXT_COMPLETE) {
				pad(out,2, scratch);
				g_string_append_printf(out, "</body>\n</html>");
			}
#ifdef DEBUG_ON
	fprintf(stderr, "closed HTML document\n");
#endif
			break;
		case LATEX_FORMAT:
			print_latex_node_tree(out, list, scratch);
			break;
		case MEMOIR_FORMAT:
			print_memoir_node_tree(out, list, scratch);
			break;
		case BEAMER_FORMAT:
			print_beamer_node_tree(out, list, scratch);
			break;
		case MAN_FORMAT:
			print_man_node_tree(out, list, scratch);
			break;
		case OPML_FORMAT:
#ifdef DEBUG_ON
	fprintf(stderr, "export OPML\n");
#endif
			begin_opml_output(out, list, scratch);
			print_opml_node_tree(out, list, scratch);
			end_opml_output(out, list, scratch);
			break;
		case ODF_FORMAT:
#ifdef DEBUG_ON
	fprintf(stderr, "export ODF\n");
#endif
			begin_odf_output(out, list, scratch);
			print_odf_node_tree(out, list, scratch);
			end_odf_output(out, list, scratch);
			break;
		case CRITIC_ACCEPT_FORMAT:
			print_critic_accept_node_tree(out, list, scratch);
			break;
		case CRITIC_REJECT_FORMAT:
			print_critic_reject_node_tree(out, list, scratch);
			break;
		case CRITIC_HTML_HIGHLIGHT_FORMAT:
			print_critic_html_highlight_node_tree(out, list, scratch);
			break;
		default:
			fprintf(stderr, "Unknown export format = %d\n",format);
			exit(EXIT_FAILURE);
	}
	
	output = out->str;
	g_string_free(out, false);
	free_scratch_pad(scratch);

#ifdef DEBUG_ON
	fprintf(stderr, "finish export_node_tree\n");
#endif
	return output;
}

/* extract_references -- go through node tree and find elements we need to reference;
   e.g. links, images, citations, footnotes 
   Remove them from main parse tree */
void extract_references(node *list, scratch_pad *scratch) {
	/* TODO: Will these all be top level elements?  What about RAW?? */
	node *temp;
	node *last = NULL;
	link_data *l;
	
	while (list != NULL) {
		switch (list->key) {
			case LINKREFERENCE:
				l = list->link_data;
				temp = mk_link(list->children, l->label, l->source, l->title, l->attr);
				
				/* store copy of link reference */
				scratch->links = cons(temp, scratch->links);
				
				/* Disconnect from children so not duplicated */
				l->attr = NULL;
				
				if (last != NULL) {
					/* remove this node from tree */
					last->next = list->next;
					free_link_data(list->link_data);
					free(list);
					list = last->next;
					continue;
				} else {
				}
				break;
			case NOTESOURCE:
				if (last != NULL) {
					last->next = list->next;
					scratch->notes = cons(list, scratch->notes);
					list = last->next;
					continue;
				}
				break;
			case GLOSSARYSOURCE:
				if (last != NULL) {
					last->next = list->next;
					scratch->notes = cons(list, scratch->notes);
					list = last->next;
					continue;
				}
				break;
			case H1: case H2: case H3: case H4: case H5: case H6:
				if ((list->children->key != AUTOLABEL) && !(scratch->extensions & EXT_NO_LABELS)) {
					char *label = label_from_node_tree(list->children);

					/* create a label from header */
					temp = mk_autolink(label);
					scratch->links = cons(temp, scratch->links);
					free(label);
				}
				break;
			case TABLE:
				if (list->children->key != TABLELABEL) {
					char *label = label_from_node(list->children);

					/* create a label from header */
					temp = mk_autolink(label);
					scratch->links = cons(temp, scratch->links);
					free(label);
				}

				break;
			case HEADINGSECTION:
				extract_references(list->children, scratch);
				break;
			default:
				break;
		}
		last = list;
		list = list->next;
	}
}

/* extract_link_data -- given a label, parse the link data and return */
link_data * extract_link_data(char *label, scratch_pad *scratch) {
	char *temp;
	link_data *d;
	node *ref = scratch->links;
	bool debug = 0;

	if (debug)
		fprintf(stderr, "try to extract link for '%s'\n",label);
	
	if ((label == NULL) || (strlen(label) == 0))
		return NULL;
	
	temp = clean_string(label);
	
	/* look for label string as is */
	while (ref != NULL) {
		if (ref->key == KEY_COUNTER) {
			ref = ref->next;
			
			continue;
		}
		if (strcmp(ref->link_data->label, temp) == 0) {
			if (debug)
				fprintf(stderr,"a:matched %s to %s\n",ref->link_data->label, label);
			/* matched */
			d = ref->link_data;
			d = mk_link_data(d->label, d->source, d->title, d->attr);
			free(temp);
			return d;
		} else {
			if (debug)
				fprintf(stderr,"a:did not match %s to %s\n",ref->link_data->label, label);
		}
		ref = ref->next;
	}
	free(temp);
	
	/* No match.  Check for label()version */
	
	if (scratch->extensions & EXT_COMPATIBILITY) {
		/* not in compat mode */
		return NULL;
	}
	temp = label_from_string(label);
	
	ref = scratch->links;
	
	while (ref != NULL) {
		if (ref->key == KEY_COUNTER) {
			ref = ref->next;
			
			continue;
		}
		if (strcmp(ref->link_data->label, temp) == 0) {
			if (debug)
				fprintf(stderr,"b:matched %s to %s\n",ref->link_data->label, label);
			/* matched */
			d = ref->link_data;
			d = mk_link_data(d->label, d->source, d->title, d->attr);
			free(temp);
			return d;
		} else {
			if (debug)
				fprintf(stderr,"b:did not match %s to %s\n",ref->link_data->label, label);
		}
		ref = ref->next;
	}
	free(temp);

	if (debug)
		fprintf(stderr, "finish extract\n");
	return NULL;
}

/* pad -- ensure that at least 'x' newlines are at end of output */
void pad(GString *out, int num, scratch_pad *scratch) {
	while (num-- > scratch->padded)
		g_string_append_c(out, '\n');
	
	scratch->padded = num;
}

/* note_number_for_label -- given a label to match, determine number to be used*/
int note_number_for_label(char *text, scratch_pad *scratch) {
	node *n = NULL;
	char *clean;
	char *label;
#ifdef DEBUG_ON
	fprintf(stderr, "find note number for: %s\n",text);
#endif

	if ((text == NULL) || (strlen(text) == 0))
		return 0;	/* Nothing to find */
	
	clean = clean_string(text);
	label = label_from_string(clean);
	
	/* have we used this note already? */
	
	/* look for label string as is */
	n = node_matching_label(clean, scratch->used_notes);
	
	/* if not, look in reserve queue */
	if (n == NULL) {
		n = node_matching_label(clean, scratch->notes);
		
		if (n != NULL) {
			/* move to used queue */
			move_note_to_used(n, scratch);
		}
	}
	
	/* Check label version */
	if (n == NULL)
		n = node_matching_label(label, scratch->used_notes);
	
	if (n == NULL) {
		n = node_matching_label(label, scratch->notes);
		
		if (n != NULL) {
			/* move to used queue */
			move_note_to_used(n, scratch);
		}
	}
	
	/* CAN recursively drill down to start counter at 0 and ++ */
	/* if found, move to used queue and return the number  */
	
	free(label);
	free(clean);
	if (n != NULL)
		return count_node_from_end(n);
	else 
		return 0;
}

/* node_matching_label -- given a string, return the node matching the string */
node * node_matching_label(char *label, node *n) {
	while (n != NULL) {
		if (n->key == KEY_COUNTER) {
			n = n->next;
			continue;
		}
		if (strcmp(n->str, label) == 0) {
			return n;
		}
		n = n->next;
	}
	
	return NULL;
}

/* since lists are stored in reverse order, need to count from end */
int count_node_from_end(node *n) {
	if (n->next == NULL) {
		if (n->key == KEY_COUNTER)
			return 0;
		return 1;	/* reserve 0 for not found */
	}
	return (count_node_from_end(n->next) + 1);
}

/* since lists are stored in reverse order, need to count from end 
	Only count cites (not footnotes) */
int cite_count_node_from_end(node *n) {
	if (n->next == NULL) {
		/* we're the last node */
		if (n->key == CITATIONSOURCE)
			return 1;
		return 0;	/* reserve 0 for not found */
	}
	if (n->key == CITATIONSOURCE) {
		return (cite_count_node_from_end(n->next) + 1);
	} else {
		return (cite_count_node_from_end(n->next));
	}
}

/* node_for_count -- given a number, get that node */
node * node_for_count(node *n, int count) {
	if (n == NULL)
		return NULL;
	
	int total = count_node_from_end(n);
	
	if (count > total)
		return NULL;
	
	if (count == total)
		return n;
	
	while (total > count) {
		n = n->next;
		if (n == NULL)
			return NULL;
		total--;
	}
	
	return n;
}

/* move_note_to_used -- snip note from ->notes and move to used_notes */
void move_note_to_used(node *list, scratch_pad *scratch) {
	node * n = scratch->notes;
	node * last = NULL;
	
	while (n != NULL) {
		if (n == list) {
			if (last != NULL) {
				last->next = n->next;
			} else {
				scratch->notes = n->next;
			}
			scratch->used_notes = cons(n, scratch->used_notes);
			return;
		}
		last = n;
		n = n->next;
	}
}

/* find attribute, if present */
node * node_for_attribute(char *querystring, node *list) {
#ifdef DEBUG_ON
	fprintf(stderr, "start node_for_attribute\n");
#endif
    node *step = NULL;
    step = list;
    char *query;
	
	if (querystring == NULL)
		return NULL;

    query = label_from_string(querystring);
#ifdef DEBUG_ON
	fprintf(stderr, "node_for_attribute 2: '%s'\n",query);
#endif
    
    while (step != NULL) {
        if ((step->str != NULL) && (strcmp(step->str,query) == 0)) {
            free(query);
#ifdef DEBUG_ON
	fprintf(stderr, "matched node_for_attribute\n");
#endif
            return step;
        }
#ifdef DEBUG_ON
	fprintf(stderr, "'%s' doesn't match '%s'\n",query,step->str);
	if (step->next == NULL) 
		fprintf(stderr, "no next node\n");
#endif
        step = step->next;
    }
    free(query);
#ifdef DEBUG_ON
	fprintf(stderr, "stop node_for_attribute\n");
#endif
    return NULL;
}


/* convert attribute to dimensions suitable for LaTeX or ODF */
/* returns c string that needs to be freed */
char * dimension_for_attribute(char *querystring, node *list) {
#ifdef DEBUG_ON
	fprintf(stderr, "start dimension_for_attribute\n");
#endif
    node *attribute;
    char *dimension;
    char *ptr;
    int i;
    char *upper;
    GString *result;

    attribute = node_for_attribute(querystring, list);
    if (attribute == NULL) return NULL;
#ifdef DEBUG_ON
	fprintf(stderr, "a\n");
#endif

    dimension = strdup(attribute->children->str);
    upper = strdup(attribute->children->str);

    for(i = 0; dimension[ i ]; i++)
        dimension[i] = tolower(dimension[ i ]);

    for(i = 0; upper[ i ]; i++)
        upper[i] = toupper(upper[ i ]);
#ifdef DEBUG_ON
	fprintf(stderr, "b\n");
#endif

    if (strstr(dimension, "px")) {
        ptr = strstr(dimension,"px");
        ptr[0] = '\0';
        strcat(ptr,"pt");
    }

    result = g_string_new(dimension);
    
    if ((strcmp(dimension,upper) == 0) && (dimension[strlen(dimension) -1] != '%')) {
        /* no units */
        g_string_append_printf(result, "pt");
    }

    free(upper);
    free(dimension);
    
    dimension = result->str;
    g_string_free(result, false);
#ifdef DEBUG_ON
	fprintf(stderr, "finish dimension_for_attribute\n");
#endif
    return(dimension);
}
