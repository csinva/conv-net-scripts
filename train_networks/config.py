DEBUG = False
SAVE_IMAGES = False

# hyperparameters
base_learning_rate = 5e-5
fmap_start = 40

# data prep paramaters
mask_threshold = 0.5
mask_dilation_steps = 1
using_in_memory = False
minimum_component_size = 1000

# runtime settings
training_gpu_device = 0
testing_gpu_device = training_gpu_device

