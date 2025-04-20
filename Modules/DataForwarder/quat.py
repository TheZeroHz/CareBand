import serial
import json

# Connect to the USB serial port
ser = serial.Serial('COM4', 115200)  # Replace 'COM4' with your actual port

# Read data
data_list = []
for _ in range(100):  # Read 100 samples
    line = ser.readline().decode('utf-8').strip()
    try:
        data_point = json.loads(line)
        data_list.append(data_point)
    except json.JSONDecodeError:
        print("Invalid JSON data received:", line)

ser.close()

import io
import json

# Create Edge Impulse compatible JSON payload
edge_impulse_samples = []
for i, data in enumerate(data_list):
    sample_data = {
        "protected": {
            "ver": "v1",
            "alg": "none",
        },
        "signature": 0,
        "payload": {
            "device_name": "CareBand",
            "device_type": "HealthCare",
            "interval_ms": 10,  # Adjust based on your sampling rate
            "sensors": [
                { "name": "qw", "units": "none" },
                { "name": "qx", "units": "none" },
                { "name": "qy", "units": "none" },
                { "name": "qz", "units": "none" },
                { "name": "ax", "units": "m/s2" },
                { "name": "ay", "units": "m/s2" },
                { "name": "az", "units": "m/s2" }
            ],
            "values": [
                [
                    data.get('qw', 0),
                    data.get('qx', 0),
                    data.get('qy', 0),
                    data.get('qz', 0),
                    data.get('ax', 0),
                    data.get('ay', 0),
                    data.get('az', 0)
                ]
            ]
        }
    }
    
edge_impulse_samples.append({
        "filename": f"imu_sample_{i+1}.json",
        "data": io.BytesIO(json.dumps(sample_data).encode('utf-8')),
        "category": "training",
        "label": "idle",  # Adjust label as needed
        "metadata": {
            "source": "USB serial",
            "collection_date": "2024-01-01"  # Adjust date as needed
        },
    })
    
from edgeimpulse import EdgeImpulse

# Initialize Edge Impulse API client
ei = EdgeImpulse(api_key='ei_60596645d94045dd34f806ff034c09eba6ab66f19de187e9c29d5292cfe02022')  # Replace with your API key

# Wrap the samples in instances of the Sample class
samples = [ei.data.sample_type.Sample(**i) for i in edge_impulse_samples]

# Upload samples to your project
response = ei.experimental.data.upload_samples(samples)

# Check if upload was successful
if len(response.fails) == 0:
    print('All data uploaded successfully')
else:
    print(f'Failed to upload {len(response.fails)} files')

# Save sample IDs for later use
ids = [sample.sample_id for sample in response.successes]