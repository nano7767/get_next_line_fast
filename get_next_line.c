/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   get_next_line.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: svikornv <marvin@42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/04/05 17:02:52 by svikornv          #+#    #+#             */
/*   Updated: 2023/05/12 14:18:21 by svikornv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "get_next_line.h"

void	init_vars(t_vars *v)
{
	v->buf = (char *)malloc(sizeof(char) * BUFFER_SIZE + 1);
	if (!v->buf)
	{
		free(v->buf);
		return ;
	}
	v->nl_node = NULL;
	v->nl_nodeth = 0;
	v->nl_indx = -1;
	v->node_count = 0;
	v->lastndlen = 0;
	v->total_len = 0;
}

int	contain_nl(t_list *stash, t_vars *v)
{
	int		i;
	t_list		*ptr;

	ptr = stash;
	while (ptr)
	{
		i = 0;
		while (ptr->content[i])
			if (ptr->content[i++] == '\n')
			{
				v->nl_node = ptr;
				v->nl_indx = i;
				return (1);
			}
		ptr = ptr->next;
		v->nl_nodeth++;
	}
	return (0);
}

void	add_to_stash(t_list **stash, t_vars *v)
{
	t_list *new_node;
	t_list	*ptr;
	int	i;

	i = 0;
	new_node = generate_node(v->read_size);
	while (i < v->read_size)
	{
		new_node->content[i] = v->buf[i];
		i++;
	}
	if (i != 0)
		v->lastndlen = i;
	new_node->content[v->read_size] = '\0';
	ptr = *stash;
	if (*stash == NULL)
	{
		*stash = new_node;
		return ;
	}
	while (ptr->next != NULL)
		ptr = ptr->next;
	ptr->next = new_node;
	v->node_count++;
}

char	*extract_line(t_list *stash, t_vars *v)
{
	int		i;
	int		j;
	char	*line;
	t_list	*ptr;

	v->total_len = (BUFFER_SIZE * v->node_count - 1) + v->lastndlen;
	line = (char *)malloc(sizeof(char) * (v->total_len + 1));
	if (!line)
		return (NULL);
	i = 0;
	ptr = stash;
	while (ptr)
	{
		j = 0;
		while (ptr->content[j] && ptr->content[j] != '\n')
			line[i++] = ptr->content[j++];
		if (ptr->content[j] == '\n')
		{
			line[i++] = '\n';
			break ;
		}
		ptr = ptr->next;
	}
	line[i] = '\0';
	return (line);
}

void	free_stash(t_list **stash, t_vars *v)
{
	t_list	*tmp;
	t_list	*ptr;
	int	i;
	int	j;

	i = v->nl_indx;
	if (v->nl_indx == -1)
	{
		clear_list(stash);
		return ;
	}
	tmp = generate_node(v->total_len - (((v->nl_nodeth - 1) * BUFFER_SIZE) + v->nl_indx + 1));
	ptr = *stash;
	j = 0;
	while (ptr)
	{
		while (ptr->content[i])
			tmp->content[j++] = ptr->content[i++];
		ptr = ptr->next;
		i = 0;
	}
	tmp->content[j] = '\0';
	clear_list(stash);
	*stash = tmp;
	v->node_count -= v->nl_nodeth;
	v->lastndlen = v->total_len - ((v->node_count - 1) * BUFFER_SIZE);
}

char	*get_next_line(int fd)
{
	static t_list	*stash = NULL;
	t_vars	v;
	char		*line;

	init_vars(&v);
	while (contain_nl(stash, &v) == 0)
	{
		v.read_size = read(fd, v.buf, BUFFER_SIZE);
		if ((!stash && !v.read_size) || v.read_size == -1 || BUFFER_SIZE <= 0)
			return (free(v.buf), NULL);
		if (v.read_size == 0 && stash && contain_nl(stash, &v) == 0)
		{
			free(v.buf);
			break ;
		}
		add_to_stash(&stash, &v);
	}
	line = extract_line(stash, &v);
	free_stash(&stash, &v);
	if (line[0] == '\0')
		return (free(line), NULL);
	return (line);		
}
/*
#include <fcntl.h>
int	main(void)
{
	int	fd;
	char	*s;

	fd = open("test.txt", O_RDONLY);
	while (s != NULL)
	{
		s = get_next_line(fd);
		printf("%s", s);
	}
}
*/
