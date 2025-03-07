/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   routine.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rbardet- <rbardet-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/01 03:12:39 by rbardet-          #+#    #+#             */
/*   Updated: 2025/03/07 10:10:09 by rbardet-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/philo.h"

long long	get_timestamp(void)
{
	struct timeval	time;

	if (gettimeofday(&time, NULL) == -1)
		write(2, "Error while getting actual time\n", 33);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

void	print_status(t_philo *philo, char *status)
{
	pthread_mutex_lock(&philo->rules->write_lock);
	if ((!alive_state(philo) || !all_alive(philo)) && ft_strcmp(status, "died"))
	{
		pthread_mutex_unlock(&philo->rules->write_lock);
		return ;
	}
	printf("%5lli " "%3d " "%s\n", get_timestamp()
		- philo->rules->start_time, philo->id, status);
	pthread_mutex_unlock(&philo->rules->write_lock);
}

void	eating(t_philo *philo)
{
	pthread_mutex_lock(philo->left_fork);
	print_status(philo, "has taken a left fork");
	if (philo->rules->nb_philo == 1)
	{
		pthread_mutex_unlock(philo->left_fork);
		return ;
	}
	pthread_mutex_lock(philo->right_fork);
	print_status(philo, "has taken a right fork");
	print_status(philo, "is eating");
	philo->last_meal = get_timestamp();
	philo->time_eaten++;
	usleep(philo->rules->time_to_eat * 1000);
	pthread_mutex_unlock(philo->left_fork);
	pthread_mutex_unlock(philo->right_fork);
}

void	philo_routine(t_philo *philo)
{
	if (philo->id % 2 == 0)
		usleep(500);
	while (alive_state(philo) && all_alive(philo))
	{
		eating(philo);
		if (philo->rules->nb_philo == 1)
		{
			usleep(philo->rules->time_to_die * 1000);
			break ;
		}
		if (philo->time_eaten == philo->rules->number_of_meal)
			return ;
		print_status(philo, "is sleeping");
		usleep(philo->rules->time_to_sleep * 1000);
		print_status(philo, "is thinking");
	}
	pthread_mutex_lock(&philo->rules->dead_lock);
	if (*philo->first_dead == 1)
	{
		pthread_mutex_unlock(&philo->rules->dead_lock);
		return ;
	}
	*philo->first_dead = 1;
	pthread_mutex_unlock(&philo->rules->dead_lock);
	print_status(philo, "died");
}

void	create_thread(pthread_mutex_t **fork, t_rules *rules)
{
	int	i;

	i = 0;
	while (i < rules->nb_philo)
	{
		if (pthread_create(&rules->philo[i].thread, NULL, (void *)philo_routine,
				&rules->philo[i]) != 0)
		{
			free_all(fork, rules);
			write(2, "Error while initing philo thread\n", 34);
			exit (EXIT_FAILURE);
		}
		i++;
	}
	i = 0;
	while (i < rules->nb_philo)
	{
		if (pthread_join(rules->philo[i].thread, NULL) != 0)
		{
			write(2, "Error while joining thread\n", 28);
			return ;
		}
		i++;
	}
}
