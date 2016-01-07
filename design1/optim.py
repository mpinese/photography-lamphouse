import subprocess
import random
import tempfile
import shelve
import math
import os
import copy



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
		self._fitness = None


	def randomize(self):
		self.genotype.randomize()
		# Phenotype is now invalid and must be regenerated
		self._phenotype = None
		self._fitness = None


	def mutate(self):
		self.genotype.mutate()
		# Phenotype is now invalid and must be regenerated
		self._phenotype = None
		self._fitness = None


	def mutate_unsafe(self):
		self.genotype.mutate_unsafe()
		# Phenotype is now invalid and must be regenerated
		self._phenotype = None
		self._fitness = None


	def getPhenotype(self):
		self.measurePhenotype()
		return self._phenotype


	def measurePhenotype(self):
		if self._phenotype != None:
			return

		rad_file = tempfile.NamedTemporaryFile(delete = False)
		oct_file = tempfile.NamedTemporaryFile(delete = False)
		grid_file = tempfile.NamedTemporaryFile(delete = False)

		m4_param_defs = self.genotype.makeM4Defstring()

		rad_data = subprocess.check_output(['m4'] + m4_param_defs + ['design.rad.m4'])
		rad_file.write(rad_data)
		rad_file.close()

		oct_tree = subprocess.check_output(['oconv', rad_file.name])
		oct_file.write(oct_tree)
		oct_file.close()

		grid_xdim = 20
		grid_zdim = 20

		for x in range(grid_xdim):
			for z in range(grid_zdim):
				grid_file.write('{}\t0\t{}\t0\t1\t0\n'.format(
					545 + (300-1)*(x/(grid_xdim-1)-0.5)*2,
					9 + (325-1)*(z/(grid_zdim-1)-0.5)*2))
		grid_file.seek(0)

		radiance_triplets = subprocess.check_output(['rtrace', '-ab', '8', '-h', oct_file.name], stdin = grid_file)
		grid_file.close()

		radiance_values = [float(line.split('\t')[0]) for line in radiance_triplets.splitlines()]

		os.unlink(rad_file.name)
		os.unlink(oct_file.name)
		os.unlink(grid_file.name)

		self._phenotype = radiance_values


	def getFitness(self):
		self.calculateFitness()
		return self._fitness


	def calculateFitness(self):
		if self._fitness != None:
			return
		radiance_values = self.getPhenotype()

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


	def getScaledFitness(self):
		raw_fitness = [i.getFitness() for i in self.individuals]
		min_fitness = min(raw_fitness)
		max_fitness = max(raw_fitness)
		range_fitness = max_fitness - min_fitness
		scaled_fitness = [f - min_fitness for f in raw_fitness]
		if range_fitness != 0:
			scaled_fitness = [f / range_fitness for f in scaled_fitness]
		return scaled_fitness


	def doGeneration(self, keep_best_frac):
		indiv_fitness = zip(self.individuals, self.getScaledFitness())
		indiv_fitness_sorted = sorted(indiv_fitness, key = lambda x: -x[1])

		n = len(self.individuals)
		m = max(2, int(n * keep_best_frac))

		best_m_of_generation = [indiv_fitness[i][0] for i in range(m)]

		self.individuals = best_m_of_generation
		for i in range(n - m):
			first_run = True
			child = None
			while first_run == True or child.genotype.checkConstraints() == False:
				first_run = False
				parents = random.sample(self.individuals[:m], 2)
				child = crossover(parents[0], parents[1])
				child.mutate_unsafe()

			self.individuals.append(child)



def crossover(individual1, individual2):
	child = Individual()
	for field in Genotype._FIELDS.keys():
		if random.random() < 0.5:
			child.genotype.gt[field] = individual1.genotype.gt[field]
		else:
			child.genotype.gt[field] = individual2.genotype.gt[field]
	return child


def doOptim(population_size, keep_best_frac, n_iters, shelf):
	if len(shelf) == 0:
		start_gen = 0
		print 'Starting new optimization'
		population = Population(population_size)
		population.randomize()
		print 'Generated population of size {}'.format(population_size)
	else:
		start_gen = max([int(x) for x in shelf.keys()])
		print 'Continuing saved optimization from generation {}'.format(start_gen)
		population = shelf[`start_gen`]
	
	for i in range(start_gen, n_iters + 1):
		shelf[`i`] = population

		population_raw_fitness = [ind.getFitness() for ind in population.individuals]
		print('Generation {}: best fitness {}, average fitness {}'.format(i, max(population_raw_fitness), sum(population_raw_fitness) / len(population)))

		new_population = copy.deepcopy(population)
		new_population.doGeneration(keep_best_frac)
		population = new_population

	return population


if __name__ == '__main__':
	shelf = shelve.open('optim.shl')
	doOptim(100, 0.2, 200, shelf)
