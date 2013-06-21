/*

	man.c -- Man writer

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

#include "man.h"


/* print_man_node_tree -- convert node tree to Man */
void print_man_node_tree(GString *out, node *list, scratch_pad *scratch) {
	while (list != NULL) {
		print_man_node(out, list, scratch);
		list = list->next;
	}
}

/* print_man_node -- convert given node to Man and append */
void print_man_node(GString *out, node *n, scratch_pad *scratch) {
	node *temp_node;
	char *temp;
	int lev;
	char *width = NULL;
	char *height = NULL;
	GString *temp_str;
	GString *raw_str;
	int i;
	double temp_float;

	if (n == NULL)
		return;
	
	/* debugging statement */
#ifdef DEBUG_ON
	fprintf(stderr, "print_man_node: %d\n",n->key);
#endif
	
	/* If we are forcing a complete document, and METADATA isn't the first thing,
		we need to close <head> */
	if ((scratch->extensions & EXT_COMPLETE)
		&& !(scratch->extensions & EXT_HEAD_CLOSED) && 
		!((n->key == FOOTER) || (n->key == METADATA))) {
			pad(out, 2, scratch);
			scratch->extensions = scratch->extensions | EXT_HEAD_CLOSED;
		}
	switch (n->key) {
		case NO_TYPE:
			break;
		case LIST:
			print_man_node_tree(out,n->children,scratch);
			break;
		case STR:
			print_man_string(out,n->str, scratch);
			break;
		case SPACE:
			g_string_append_printf(out,"%s",n->str);
			break;
		case PLAIN:
			pad(out,1, scratch);
			print_man_node_tree(out,n->children, scratch);
			scratch->padded = 0;
			break;
		case PARA:
			pad(out, 2, scratch);
			print_man_node_tree(out,n->children,scratch);
			scratch->padded = 0;
			break;
		case HRULE:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{center}\\rule{3in}{0.4pt}\\end{center}\n");
			scratch->padded = 0;
			break;
		case HTMLBLOCK:
			/* don't print HTML block */
			/* but do print HTML comments for raw Man */
			if (strncmp(n->str,"<!--",4) == 0) {
				pad(out, 2, scratch);
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "%s", &n->str[4]);
				scratch->padded = 0;
			}
			break;
		case VERBATIM:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{verbatim}\n%s\\end{verbatim}",n->str);
			scratch->padded = 0;
			break;
		case BULLETLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{itemize}");
			scratch->padded = 0;
			print_man_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\end{itemize}");
			scratch->padded = 0;
			break;
		case ORDEREDLIST:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{enumerate}");
			scratch->padded = 0;
			print_man_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\end{enumerate}");
			scratch->padded = 0;
			break;
		case LISTITEM:
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\item ");
			scratch->padded = 2;
			print_man_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\n");
			scratch->padded = 0;
			break;
		case METADATA:
			break;
		case METAKEY:
			break;
		case METAVALUE:
			trim_trailing_whitespace(n->str);
			print_man_string(out,n->str, scratch);
			break;
		case FOOTER:
			break;
		case HEADINGSECTION:
			print_man_node_tree(out,n->children,scratch);
			break;
		case H1: case H2: case H3: case H4: case H5: case H6:
			lev = n->key - H1 + scratch->baseheaderlevel;  /* assumes H1 ... H6 are in order */
			if (lev > 7)
				lev = 7;	/* Max at level 7 */
			pad(out, 2, scratch);
			if (lev == H1) {
			    g_string_append_printf(out, ".SH \"");
			}
			else {
			    g_string_append_printf(out, ".SS \"");
			}
			print_man_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\"\n");
			scratch->padded = 0;
			break;
		case APOSTROPHE:
			print_man_localized_typography(out, APOS, scratch);
			break;
		case ELLIPSIS:
			print_man_localized_typography(out, ELLIP, scratch);
			break;
		case EMDASH:
			print_man_localized_typography(out, MDASH, scratch);
			break;
		case ENDASH:
			print_man_localized_typography(out, NDASH, scratch);
			break;
		case SINGLEQUOTED:
			print_man_localized_typography(out, LSQUOTE, scratch);
			print_man_node_tree(out, n->children, scratch);
			print_man_localized_typography(out, RSQUOTE, scratch);
			break;
		case DOUBLEQUOTED:
			print_man_localized_typography(out, LDQUOTE, scratch);
			print_man_node_tree(out, n->children, scratch);
			print_man_localized_typography(out, RDQUOTE, scratch);
			break;
		case LINEBREAK:
			g_string_append_printf(out, "\\\\\n");
			break;
		case MATHSPAN:
			break;
		case STRONG:
			g_string_append_printf(out, "\n.B ");
			print_man_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\n");
			break;
		case EMPH:
			g_string_append_printf(out, "\n.I ");
			print_man_node_tree(out,n->children,scratch);
			g_string_append_printf(out, "\n");
			break;
		case LINKREFERENCE:
			break;
		case LINK:
			break;
		case ATTRKEY:
			g_string_append_printf(out, " %s=\"%s\"", n->str,
				n->children->str);
			break;
		case REFNAME:
		case SOURCE:
		case TITLE:
			break;
		case IMAGEBLOCK:
			pad(out,2, scratch);
		case IMAGE:
#ifdef DEBUG_ON
	fprintf(stderr, "print image\n");
#endif
			/* Do we have proper info? */
			if ((n->link_data->label == NULL) &&
			(n->link_data->source == NULL)) {
				/* we seem to be a [foo][] style link */
				/* so load a label */
				temp_str = g_string_new("");
				print_raw_node_tree(temp_str, n->children);
				n->link_data->label = temp_str->str;
				g_string_free(temp_str, FALSE);
			}
			/* Load reference data */
			if (n->link_data->label != NULL) {
				temp = strdup(n->link_data->label);
				free_link_data(n->link_data);
				n->link_data = extract_link_data(temp, scratch);
				if (n->link_data == NULL) {
					/* replace original text since no definition found */
					g_string_append_printf(out, "![");
					print_man_node(out, n->children, scratch);
					g_string_append_printf(out,"]");
					if (n->children->next != NULL) {
						g_string_append_printf(out, "[");
						print_man_node_tree(out, n->children->next, scratch);
						g_string_append_printf(out,"]");
					} else if (n->str != NULL) {
						/* no title label, so see if we stashed str*/
						g_string_append_printf(out, "%s", n->str);
					} else {
						g_string_append_printf(out, "[%s]",temp);
					}
					free(temp);
					break;
				}
				free(temp);
			}
			
			if (n->key == IMAGEBLOCK)
				g_string_append_printf(out, "\\begin{figure}[htbp]\n\\centering\n");

			g_string_append_printf(out, "\\includegraphics[");

#ifdef DEBUG_ON
	fprintf(stderr, "attributes\n");
#endif

			if (n->link_data->attr != NULL) {
				temp_node = node_for_attribute("height",n->link_data->attr);
				if (temp_node != NULL)
					height = correct_dimension_units(temp_node->children->str);
				temp_node = node_for_attribute("width",n->link_data->attr);
				if (temp_node != NULL)
					width = correct_dimension_units(temp_node->children->str);
			}
			
			if ((height == NULL) && (width == NULL)) {
				/* No dimensions used */
				g_string_append_printf(out, "keepaspectratio,width=\\textwidth,height=0.75\\textheight");
			} else {
				/* At least one dimension given */
				if (!((height != NULL) && (width != NULL))) {
					/* we only have one */
					g_string_append_printf(out, "keepaspectratio,");
				}
				
				if (width != NULL) {
					if (width[strlen(width)-1] == '%') {
						width[strlen(width)-1] = '\0';
						temp_float = strtod(width, NULL);
						temp_float = temp_float/100;
						g_string_append_printf(out, "width=$.4f\\textwidth,", temp_float);
					} else {
						g_string_append_printf(out, "width=%s,",width);
					}
				} else {
					g_string_append_printf(out, "width=\\textwidth,");
				}
				
				if (height != NULL) {
					if (height[strlen(height)-1] == '%') {
						height[strlen(height)-1] = '\0';
						temp_float = strtod(height, NULL);
						temp_float = temp_float/100;
						g_string_append_printf(out, "height=$.4f\\textheight", temp_float);
					} else {
						g_string_append_printf(out, "height=%s",height);
					}
				} else {
					g_string_append_printf(out, "height=0.75\\textheight");
				}
			}

			g_string_append_printf(out, "]{%s}",n->link_data->source);
			
			if (n->key == IMAGEBLOCK) {
				if (n->children != NULL) {
					g_string_append_printf(out, "\n\\caption{");
					print_man_node_tree(out, n->children, scratch);
					g_string_append_printf(out, "}");
				}
				if (n->link_data->label != NULL)
					g_string_append_printf(out, "\n\\label{%s}", n->link_data->label);
				g_string_append_printf(out, "\n\\end{figure}");
				scratch->padded = 0;
			}
			
			free(height);
			free(width);
			n->link_data->attr = NULL;	/* We'll delete these elsewhere */
			break;
#ifdef DEBUG_ON
	fprintf(stderr, "finish image\n");
#endif
		case NOTEREFERENCE:
			lev = note_number_for_label(n->str, scratch);
			temp_node = node_for_count(scratch->used_notes, lev);
			scratch->padded = 2;
			if (temp_node->key == GLOSSARYSOURCE) {
				g_string_append_printf(out, "\\newglossaryentry{%s}{",temp_node->children->children->str);
				print_man_node_tree(out, temp_node->children, scratch);
				g_string_append_printf(out, "}}\\glsadd{%s}",temp_node->children->children->str);
			} else {
				g_string_append_printf(out, "\\footnote{");
				print_man_node_tree(out, temp_node->children, scratch);
				g_string_append_printf(out, "}");
			}
			scratch->padded = 0;
			break;
		case NOCITATION:
		case CITATION:
#ifdef DEBUG_ON
	fprintf(stderr, "\nprint cite\n");
#endif
			if ((n->link_data != NULL) && (strncmp(n->link_data->label,"[#",2) == 0)) {
				/* external citation (e.g. BibTeX) */
				n->link_data->label[strlen(n->link_data->label)-1] = '\0';
				if (n->key == NOCITATION) {
					g_string_append_printf(out, "~\\nocite{%s}",&n->str[2]);
				} else {
					g_string_append_printf(out, "<FAKE span class=\"externalcitation\">");
					g_string_append_printf(out, "</span>");
				}
			} else {
#ifdef DEBUG_ON
				fprintf(stderr, "internal cite\n");
#endif
				/* MMD citation, so output as footnote */
				/* TODO: create separate stream from footnotes */
				lev = note_number_for_label(n->link_data->label, scratch);
				if (lev != 0) {
#ifdef DEBUG_ON
					fprintf(stderr, "matching cite found\n");
#endif
					temp_node = node_for_count(scratch->used_notes, lev);
					/* flag that this is used as a citation */
					temp_node->key = CITATIONSOURCE;
					if (lev > scratch->max_footnote_num) {
						scratch->max_footnote_num = lev;
					}
					if (n->key == NOCITATION) {
						g_string_append_printf(out, "~\\nocite{%s}", n->link_data->label);
					} else {
						if (n->children != NULL) {
							g_string_append_printf(out, "~\\citep[");
							print_man_node(out, n->children, scratch);
							g_string_append_printf(out,"]{%s}",n->link_data->label);
						} else {
							g_string_append_printf(out, "~\\citep{%s}", n->link_data->label);
						}
					}
				} else {
					/* not located -- this is external cite */
#ifdef DEBUG_ON
				fprintf(stderr, "no match for cite: '%s'\n",n->link_data->label);
#endif
					temp = n->link_data->label;
					if (n->key == NOCITATION) {
						g_string_append_printf(out, "~\\nocite{%s}",n->link_data->label);
					} else {
						if (n->children != NULL) {
#ifdef DEBUG_ON
				fprintf(stderr, "cite with children\n");
#endif
							if (strcmp(&temp[strlen(temp) - 1],";") == 0) {
								g_string_append_printf(out, " \\citet[");
								temp[strlen(temp) - 1] = '\0';
							} else {
								g_string_append_printf(out, "~\\citep[");
							}
							print_man_node(out, n->children, scratch);
							g_string_append_printf(out, "]{%s}",temp);
						} else {
#ifdef DEBUG_ON
				fprintf(stderr, "cite without children. locat:'%s'\n",n->str);
#endif
							if (strcmp(&temp[strlen(temp) - 1],";") == 0) {
								temp[strlen(temp) - 1] = '\0';
								g_string_append_printf(out, " \\citet{%s}",temp);
							} else {
								g_string_append_printf(out, "~\\citep{%s}",temp);
							}
						}
					}
				}
			}
#ifdef DEBUG_ON
		fprintf(stderr, "finish cite\n");
#endif
			break;
		case GLOSSARYTERM:
			if ((n->next != NULL) && (n->next->key == GLOSSARYSORTKEY) ) {
				g_string_append_printf(out, "sort={");
				print_man_string(out, n->next->str, scratch);
				g_string_append_printf(out, "},");
			}
			g_string_append_printf(out,"name={");
			print_man_string(out, n->children->str, scratch);
			g_string_append_printf(out, "},description={");
			break;
		case GLOSSARYSORTKEY:
			break;
		case CODE:
			g_string_append_printf(out, "\\texttt{");
			print_man_string(out, n->str, scratch);
			g_string_append_printf(out, "}");
			break;
		case BLOCKQUOTEMARKER:
			print_man_node_tree(out, n->children, scratch);
			break;
		case BLOCKQUOTE:
			pad(out,2, scratch);
			g_string_append_printf(out, "\\begin{quote}");
			scratch->padded = 0;
			print_man_node_tree(out, n->children, scratch);
			pad(out,1, scratch);
			g_string_append_printf(out, "\\end{quote}");
			scratch->padded = 0;
			break;
		case RAW:
			/* This shouldn't happen */
			g_string_append_printf(out, "RAW:");
			g_string_append_printf(out,"%s",n->str);
			break;
		case HTML:
			/* don't print HTML block */
			/* but do print HTML comments for raw Man */
			if (strncmp(n->str,"<!--",4) == 0) {
				/* trim "-->" from end */
				n->str[strlen(n->str)-3] = '\0';
				g_string_append_printf(out, "%s", &n->str[4]);
				scratch->padded = 0;
			}
			break;
		case DEFLIST:
			pad(out,2, scratch);
			g_string_append_printf(out, "\\begin{description}");
			scratch->padded = 0;
			print_man_node_tree(out, n->children, scratch);
			pad(out, 1, scratch);
			g_string_append_printf(out, "\\end{description}");
			scratch->padded = 0;
			break;
		case TERM:
			pad(out,2, scratch);
			g_string_append_printf(out, "\\item[");
			print_man_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "]");
			scratch->padded = 0;
			break;
		case DEFINITION:
			pad(out, 2, scratch);
			scratch->padded = 2;
			print_man_node_tree(out, n->children, scratch);
			scratch->padded = 0;
			break;
		case TABLE:
			pad(out, 2, scratch);
			g_string_append_printf(out, "\\begin{table}[htbp]\n\\begin{minipage}{\\linewidth}\n\\setlength{\\tymax}{0.5\\linewidth}\n\\centering\n\\small\n");
			print_man_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\n\\end{tabulary}\n\\end{minipage}\n\\end{table}");
			scratch->padded = 0;
			break;
		case TABLESEPARATOR:
			temp = strdup(n->str);
			for (i = 0; temp[i]; i++)
				temp[i] = toupper(temp[i]);
			
			g_string_append_printf(out, "\\begin{tabulary}{\\textwidth}{@{}%s@{}} \\toprule\n", temp);
			free(temp);
			break;
		case TABLECAPTION:
			if ((n->children != NULL) && (n->children->key == TABLELABEL)) {
				temp = label_from_string(n->children->str);
			} else {
				temp = label_from_node_tree(n->children);
			}
			g_string_append_printf(out, "\\caption{");
			print_man_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "}\n\\label{%s}\n", temp);
			free(temp);
			break;
		case TABLELABEL:
			break;
		case TABLEHEAD:
			print_man_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\midrule\n");
			break;
		case TABLEBODY:
			print_man_node_tree(out, n->children, scratch);
			if ((n->next != NULL) && (n->next->key == TABLEBODY)) {
				g_string_append_printf(out, "\n\\midrule\n");
			} else {
				g_string_append_printf(out, "\n\\bottomrule\n");
			}
			break;
		case TABLEROW:
			print_man_node_tree(out, n->children, scratch);
			g_string_append_printf(out, "\\\\\n");
			break;
		case TABLECELL:
			scratch->padded = 2;
			if ((n->children != NULL) && (n->children->key == CELLSPAN)) {
				g_string_append_printf(out, "\\multicolumn{%d}{c}{",(int)strlen(n->children->str)+1);
			}
			print_man_node_tree(out, n->children, scratch);
			if ((n->children != NULL) && (n->children->key == CELLSPAN)) {
				g_string_append_printf(out, "}");
			}
			if (n->next != NULL)
				g_string_append_printf(out, "&");
			break;
		case CELLSPAN:
			break;
		case GLOSSARYSOURCE:
			print_man_node_tree(out, n->children, scratch);
			break;
		case CITATIONSOURCE:
		case NOTESOURCE:
			print_man_node(out, n->children, scratch);
			break;
		case SOURCEBRANCH:
			fprintf(stderr,"SOURCEBRANCH\n");
			break;
		case NOTELABEL:
			break;
		case KEY_COUNTER:
			break;
		default:
			fprintf(stderr, "print_man_node encountered unknown node key = %d\n",n->key);
			exit(EXIT_FAILURE);
	}
}



/* print_man_localized_typography -- convert to "smart" typography */
void print_man_localized_typography(GString *out, int character, scratch_pad *scratch) {
	if (!extension(EXT_SMART, scratch->extensions)) {
		g_string_append_c(out, character);
		return;
	}
	switch (character) {
		case LSQUOTE:
			switch (scratch->language) {
				case SWEDISH:
					g_string_append_printf(out, "'");
					break;
				case FRENCH:
					g_string_append_printf(out,"'");
					break;
				case GERMAN:
					g_string_append_printf(out,"‚");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"›");
					break;
				default:
					g_string_append_printf(out,"`");
				}
			break;
		case RSQUOTE:
			switch (scratch->language) {
				case GERMAN:
					g_string_append_printf(out,"`");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"‹");
					break;
				default:
					g_string_append_printf(out,"'");
				}
			break;
		case APOS:
			g_string_append_printf(out,"'");
			break;
		case LDQUOTE:
			switch (scratch->language) {
				case DUTCH:
				case GERMAN:
					g_string_append_printf(out,"„");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"»");
					break;
				case FRENCH:
					g_string_append_printf(out,"«");
					break;
				case SWEDISH:
					g_string_append_printf(out, "''");
					break;
				default:
					g_string_append_printf(out,"``");
				}
			break;
		case RDQUOTE:
			switch (scratch->language) {
				case SWEDISH:
				case DUTCH:
					g_string_append_printf(out,"''");
					break;
				case GERMAN:
					g_string_append_printf(out,"``");
					break;
				case GERMANGUILL:
					g_string_append_printf(out,"«");
					break;
				case FRENCH:
					g_string_append_printf(out,"»");
					break;
				default:
					g_string_append_printf(out,"''");
				}
			break;
		case NDASH:
			g_string_append_printf(out,"--");
			break;
		case MDASH:
			g_string_append_printf(out,"---");
			break;
		case ELLIP:
			g_string_append_printf(out,"{\\ldots}");
			break;
			default:;
	}
}

/* print_man_string - print string, escaping for Man */
void print_man_string(GString *out, char *str, scratch_pad *scratch) {
	char *tmp;
	if (str == NULL)
		return;
	while (*str != '\0') {
		switch (*str) {
			case '{': case '}': case '$': case '%':
			case '&': case '_': case '#':
				g_string_append_printf(out, "\\%c", *str);
				break;
			case '^':
				g_string_append_printf(out, "\\^{}");
				break;
			case '\\':
				g_string_append_printf(out, "\\textbackslash{}");
				break;
			case '~':
				g_string_append_printf(out, "\\ensuremath{\\sim}");
				break;
			case '|':
				g_string_append_printf(out, "\\textbar{}");
				break;
			case '<':
				g_string_append_printf(out, "$<$");
				break;
			case '>':
				g_string_append_printf(out, "$>$");
				break;
			case '/':
				str++;
				while (*str == '/') {
					g_string_append_printf(out, "/");
					str++;
				}
				g_string_append_printf(out, "\\slash ");
				str--;
				break;
			case '\n':
				tmp = str;
				tmp--;
				if (*tmp == ' ') {
					tmp--;
					if (*tmp == ' ') {
						g_string_append_printf(out, "\\\\\n");
					} else {
						g_string_append_printf(out, "\n");
					}
				} else {
					g_string_append_printf(out, "\n");
				}
				break;
			default:
				g_string_append_c(out, *str);
			}
		str++;
	}
}

/* print_man_url - print url, escaping for Man */
void print_man_url(GString *out, char *str, scratch_pad *scratch) {
	if (str == NULL)
		return;
	while (*str != '\0') {
		switch (*str) {
			case '$': case '%': case '!':
			case '&': case '_': case '#':
				g_string_append_printf(out, "\\%c", *str);
				break;
			case '^':
				g_string_append_printf(out, "\\^{}");
				break;
			default:
				g_string_append_c(out, *str);
			}
		str++;
	}
}

