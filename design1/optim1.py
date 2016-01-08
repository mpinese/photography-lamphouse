import subprocess
import random
import tempfile
import shelve
import math
import os
import copy
import multiprocessing
import sys
import os.path
import signal


class Genotype:
	_FIELDS = {
		'a': 'int',
		'b': 'int',
		'c': 'int',
		'd': 'int',
		'e': 'int',
		'f': 'int',
		'g': 'int',
		'h': 'int',
		'i': 'int',
		'j': 'bool',
		'k': 'bool'}

	def __init__(self):
		self.gt = {}
		for f in Genotype._FIELDS.keys():
			if Genotype._FIELDS[f] == 'int':
				self.gt[f] = 0
			elif Genotype._FIELDS[f] == 'bool':
				self.gt[f] = 'false'


	def __repr__(self):
		keys = self.gt.keys()
		keys.sort()
		return '|'.join((`self.gt[k]` for k in keys))


	def makeM4Defstring(self):
		return ['-D{}={}'.format(field, self.gt[field]) for field in Genotype._FIELDS.keys()]


	def checkConstraints(self):
		# Constraints:
		# a >= 0
		# b >= 0
		# b <= 955
		# c >= 0
		# d >= 0
		# e >= 0
		# f >= 0
		# g >= 0
		# h >= 15
		# h <= 345
		# i >= 0
		# i <= 845 + b
		# j in {true, false}
		# k in {true, false}
		# g + d + 300 <= c + e + f
		if self.gt['a'] < 0 or self.gt['b'] < 0 or self.gt['c'] < 0 or self.gt['d'] < 0 or self.gt['e'] < 0 or self.gt['f'] < 0 or self.gt['g'] < 0 or self.gt['h'] < 15 or self.gt['i'] < 0:
			return False
		if self.gt['b'] > 955 or self.gt['h'] > 345:
			return False
		if self.gt['i'] - self.gt['b'] > 845:
			return False
		if self.gt['g'] + self.gt['d'] + 300 > self.gt['c'] + self.gt['e'] + self.gt['f']:
			return False
		if self.gt['j'] != 'true' and self.gt['j'] != 'false':
			return False
		if self.gt['k'] != 'true' and self.gt['k'] != 'false':
			return False
		return True


	def randomize_unsafe(self):
		for field in Genotype._FIELDS.keys():
			if Genotype._FIELDS[field] == 'int':
				self.gt[field] = int(random.gauss(0, 1000))
			elif Genotype._FIELDS[field] == 'bool':
				self.gt[field] = random.choice(['true', 'false'])


	def mutate_unsafe(self):
		for field in Genotype._FIELDS.keys():
			if random.random() * len(self.gt) < 1:
				if Genotype._FIELDS[field] == 'int':
					self.gt[field] += int(random.gauss(0, 100))
				elif Genotype._FIELDS[field] == 'bool':
					self.gt[field] = random.choice(['true', 'false'])


	def randomize(self):
		self.randomize_unsafe()
		while self.checkConstraints() == False:
			self.randomize_unsafe()


	def mutate(self):
		original_gt = self.gt.copy()
		self.mutate_unsafe()
		while self.checkConstraints() == False:
			self.gt = original_gt.copy()
			self.mutate_unsafe()



class Individual:
	def __init__(self):
		self.genotype = Genotype()
		self._phenotype = None
		self._phenotype_ab = None
		self._fitness = None


	def __repr__(self):
		return str(self.genotype)


	def randomize(self):
		self.genotype.randomize()
		# Phenotype is now invalid and must be regenerated
		self._phenotype = None
		self._phenotype_ab = None
		self._fitness = None


	def mutate(self):
		self.genotype.mutate()
		# Phenotype is now invalid and must be regenerated
		self._phenotype = None
		self._phenotype_ab = None
		self._fitness = None


	def mutate_unsafe(self):
		self.genotype.mutate_unsafe()
		# Phenotype is now invalid and must be regenerated
		self._phenotype = None
		self._phenotype_ab = None
		self._fitness = None


	def getPhenotype(self, ab):
		self.measurePhenotype(ab)
		return self._phenotype


	def measurePhenotype(self, ab):
		if self._phenotype != None and self._phenotype_ab == ab:
			return

		rad_file = tempfile.NamedTemporaryFile(delete = False)
		oct_file = tempfile.NamedTemporaryFile(delete = False)
		grid_file = tempfile.NamedTemporaryFile(delete = False)

		m4_param_defs = self.genotype.makeM4Defstring()

		rad_data = subprocess.check_output(['m4'] + m4_param_defs + ['design1.rad.m4'])
		rad_file.write(rad_data)
		rad_file.close()

		oct_tree = subprocess.check_output(['oconv', rad_file.name])
		oct_file.write(oct_tree)
		oct_file.close()

		grid_xdim = 8
		grid_zdim = 8

		for x in range(grid_xdim):
			for z in range(grid_zdim):
				grid_file.write('{}\t0\t{}\t0\t1\t0\n'.format(
					545 + (300-1)*(x/(grid_xdim-1)-0.5)*2,
					9 + (325-1)*(z/(grid_zdim-1)-0.5)*2))
		grid_file.seek(0)

		radiance_triplets = subprocess.check_output(['rtrace', '-ab', `ab`, '-h', oct_file.name], stdin = grid_file)
		grid_file.close()

		radiance_values = [float(line.split('\t')[0]) for line in radiance_triplets.splitlines()]

		os.unlink(rad_file.name)
		os.unlink(oct_file.name)
		os.unlink(grid_file.name)

		self._phenotype = radiance_values
		self._phenotype_ab = ab


	def getFitness(self, ab):
		self.calculateFitness(ab)
		return self._fitness


	def calculateFitness(self, ab):
		if self._fitness != None and self._phenotype_ab == ab:
			return

		radiance_values = self.getPhenotype(ab)

		params = self.genotype.gt
		bb_volume = (params['a'] + params['c'] + params['e'] + params['f'] + 60.0) * (245 + 600 + params['b'] + 60.0) * (990 + 60.0) / 1000
		glass_area = 0.0
		if params['j'] == 'true':
			glass_area += (params['a'] + params['c'] + params['e'] + params['f']) * 990.0 / 100
		if params['k'] == 'true':
			glass_area += math.sqrt(math.pow(abs(params['c'] - params['d']), 2) + math.pow(245 + 600 + params['b'], 2)) * 990.0 / 100
		min_radiance = min(radiance_values)
		max_radiance = max(radiance_values)

		# Add a tiny offset for stability
		min_radiance += 1e-9
		max_radiance += 1e-9

		radiance_inhomogeneity = (max_radiance - min_radiance) / min_radiance

		# print(pow(radiance_inhomogeneity / 0.03, 5), min_radiance)

		# Objective: Radiance inhomogeneity should be < 0.03
		# Subject to this, min_radiance should be as high as possible.
		# Later, bb_volume and glass_area may be optimization targets.

		neg_fitness = pow(radiance_inhomogeneity / 0.03, 5) - min_radiance

		self._fitness = -neg_fitness



class Population:
	def __init__(self):
		self.individuals = []


	def __init__(self, n):
		self.individuals = []
		for i in range(n):
			self.individuals.append(Individual())


	def __len__(self):
		return len(self.individuals)


	def randomize(self):
		for i in self.individuals:
			i.randomize()


	def getRawFitness(self, ab):
		return [i.getFitness(ab) for i in self.individuals]


	def getScaledFitness(self, ab):
		raw_fitness = self.getRawFitness(ab)
		min_fitness = min(raw_fitness)
		max_fitness = max(raw_fitness)
		range_fitness = max_fitness - min_fitness
		scaled_fitness = [f - min_fitness for f in raw_fitness]
		if range_fitness != 0:
			scaled_fitness = [f / range_fitness for f in scaled_fitness]
		return scaled_fitness


	def doParallelPhenotypeComputation(self, ab):
		# Evaluate individual fitness.  Do this with multiprocessing, for speed.
		# Unfortunately, the class really wasn't well thought-out, and 
		# multiprocessing really needs to be shoehorned into it.  Do as follows:
		#   Multiprocess: pheno = [i.getPhenotype() for i in self.individuals]
		#   for p, i in zip(pheno, self.individuals):
		#      i._phenotype = p
		# Then scaled fitness can be calculated using Population.getScaledFitness,
		# as the called individual.calculateFitness is fast.
		# print('  SMP phenotype precomputation...')

		pool = multiprocessing.Pool(8, getPhenotypeWorkerInit)

		pheno = pool.map(getPhenotypeWorker, zip(self.individuals, [ab] * len(self.individuals)))
		for p, i in zip(pheno, self.individuals):
			i._phenotype = p
			i._phenotype_ab = ab


	def isGenotypeNew(self, indiv):
		# Returns true if indiv's genotype is *not* present within self.individuals,
		# else false.  Relies on Genotype.__repr__ being a complete and unique representation
		# of the genotype.
		for test_i in self.individuals:
			if str(test_i) == str(indiv):
				return False
		return True


	def doGeneration(self, keep_best_frac, ab):
		# Perform one generation of evaluation, selection, mating, and mutation.

		# Evaluation: calculate fitness, taking advantage of the precomputed phenotypes
		# print('  Fitness evaluation...')
		indiv_fitness = zip(self.individuals, self.getRawFitness(ab))
		indiv_fitness_sorted = sorted(indiv_fitness, key = lambda x: -x[1])
		
		# print 'GENERATION:'

		# print '  Before selection:'
		# for i in indiv_fitness_sorted:
		# 	print '    ', i[0], '\t', i[1]

		# Selection: Determine how many of the highest fitness individuals to keep (m)
		# print('  Selection...')
		n = len(self.individuals)
		m = max(2, int(n * keep_best_frac))

		# Reassign these top m individuals to self.individuals
		best_m_of_generation = [indiv_fitness_sorted[i][0] for i in range(m)]
		self.individuals = best_m_of_generation

		# print 'After selection:'
		# for i in best_m_of_generation:
		# 	print '    ', i, '\t', i.getFitness(ab)

		# Mating: Using the top m individuals as parents, generate n-m offpring 
		# to bring the population size back up to n.
		# print('  Mating...')
		# print 'New children:'
		for i in range(n - m):
			child = None
			while child == None or child.genotype.checkConstraints() == False or self.isGenotypeNew(child) == False:
				parents = random.sample(self.individuals[:m], 2)
				child = crossover(parents[0], parents[1])
				child.mutate_unsafe()

			# print '    ', child, '\t', child.getFitness(ab)
			self.individuals.append(child)


	def calcDiversity(self):
		if len(self.individuals) == 0:
			return None

		loci = self.individuals[0].genotype.gt.keys()
		n_loci = len(loci)
		n_individuals = len(self.individuals)

		locus_cv = []
		locus_mean = []
		for locus in loci:
			if Genotype._FIELDS[locus] == 'int':
				locus_data = [i.genotype.gt[locus] for i in self.individuals]
			elif Genotype._FIELDS[locus] == 'bool':
				locus_data = [0] * n_individuals
				for i in range(n_individuals):
					if self.individuals[i].genotype.gt[locus] == 'true':
						locus_data[i] = 1

			locus_mean.append(sum(locus_data) * 1.0 / n_individuals)
			locus_sd = math.sqrt(sum((math.pow(d - locus_mean[-1], 2) for d in locus_data)) / (n_individuals-1))
			if locus_mean[-1] == 0:
				locus_cv.append(float('inf'))
			else:
				locus_cv.append(locus_sd / locus_mean[-1])

		return zip(loci, zip(locus_mean, locus_cv))


def getPhenotypeWorkerInit():
    signal.signal(signal.SIGINT, signal.SIG_IGN)

def getPhenotypeWorker(task):
	return task[0].getPhenotype(ab = task[1])



def crossover(individual1, individual2):
	child = Individual()
	for field in Genotype._FIELDS.keys():
		if random.random() < 0.5:
			child.genotype.gt[field] = individual1.genotype.gt[field]
		else:
			child.genotype.gt[field] = individual2.genotype.gt[field]
	return child



def doOptim(population_size, schedule, shelf):
	# Get the last generation present in the shelf object
	last_gen = -1
	for key in shelf.keys():
		if key != 'params':
			last_gen = max(last_gen, int(key))

	# The starting generation is the one immediately following
	# the last in the file.
	start_gen = last_gen + 1

	if start_gen == 0:
		print 'Starting new optimization'
		population = Population(population_size)
		population.randomize()
		print 'Generated population of size {}'.format(population_size)
	else:
		print 'Continuing saved optimization from generation {}'.format(start_gen)
		population = shelf[`last_gen`]
	
	n_iters = len(schedule)

	for i in range(start_gen, n_iters):
		population.doParallelPhenotypeComputation(ab = schedule[i][0])
		population_raw_fitness = [ind.getFitness(ab = schedule[i][0]) for ind in population.individuals]
		population_diversity = population.calcDiversity()

		print('Generation {}: f_best {:.2e}, f_av {:.2e}, Sigma_cv {:.2e}'.format(i, max(population_raw_fitness), sum(population_raw_fitness) / len(population), sum((d[1][1] for d in population_diversity))))

		shelf[`i`] = population

		# new_population = copy.deepcopy(population)
		# new_population.doGeneration(keep_best_frac = schedule[i][1], ab = schedule[i][0])
		# population = new_population

		population.doGeneration(keep_best_frac = schedule[i][1], ab = schedule[i][0])

	return population



if __name__ == '__main__':
	if len(sys.argv) == 2:
		if not os.path.isfile(sys.argv[1]):
			sys.exit('Error: supplied shelf {} can not be found.'.format(sys.argv[1]))
		shelf = shelve.open(sys.argv[1])
		if not 'params' in shelf:
			sys.exit('Error: supplied shelf {} does not contain parameters field.'.format(sys.argv[1]))
		params = shelf['params']
		schedule = params['schedule']
	elif len(sys.argv) > 2 and len(sys.argv) % 2 == 0:
		shelf = shelve.open(sys.argv[1])
		if 'params' in shelf:
			params = shelf['params']
		else:
			params = {'popsize': int(sys.argv[2]), 'keepfrac': float(sys.argv[3]), 'schedule': []}

		new_schedule = []
		for i in range(4, len(sys.argv), 2):
			new_schedule += zip([int(sys.argv[i+1])]*int(sys.argv[i]), [params['keepfrac']]*int(sys.argv[i]))
		params['schedule'] = params['schedule'] + new_schedule
	else:
		sys.exit('Usage: optim.py <shelf> [<popsize> <keepfrac> <niter1> <ab1> [<niter2> <ab2> [<niter3> <ab3> [...]]] ]')

	shelf['params'] = params
	doOptim(params['popsize'], params['schedule'], shelf)

