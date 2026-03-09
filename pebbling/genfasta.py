import random

def mutate(sequence, mutation_rate):
    """Introduces random SNPs into a sequence."""
    dna = list("ACGT")
    new_seq = []
    for base in sequence:
        if random.random() < mutation_rate:
            new_seq.append(random.choice([b for b in dna if b != base]))
        else:
            new_seq.append(base)
    return "".join(new_seq)

# Parameters
ref_length = 1000000
num_paths = 50
mutation_rate = 0.05 # 5% divergence

# Generate random reference
reference = "".join(random.choices("ACGT", k=ref_length))

with open("simulated_pangenome.fasta", "w") as f:
    for i in range(num_paths):
        mutated_seq = mutate(reference, mutation_rate)
        # insert a \n every 80 characters for FASTA formatting
        mutated_seq = "\n".join(mutated_seq[i:i+80] for i in range(0, len(mutated_seq), 80))
        # The FASTA header becomes the path name (color) in the GFA
        f.write(f">path_{i}\n{mutated_seq}\n")