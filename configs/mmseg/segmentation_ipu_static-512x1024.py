_base_ = ['./segmentation_static.py']


backend_config = dict(type='ipu', precision='fp16', output_dir='',
                      batches_per_step=1, ipu_version='ipu21', input_shape='input=1,3,1024,512', eightbitsio='', available_memory_proportion=0.2,
                      popart_options=dict(rearrangeAnchorsOnHost='false', enablePrefetchDatastreams='false', groupHostSync='false', partialsTypeMatMuls="half"))

onnx_config = dict(input_shape=[512, 1024])
