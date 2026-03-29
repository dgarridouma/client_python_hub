import hmac
import hashlib
import base64

# Rellenar con los valores del enrollment group
GROUP_PRIMARY_KEY = "tsnQQDlxNRyhiUQLVww+rLvKsyq5dksYFIC4UrN8PvxSVOKkzrBQP/sUvm29AUbyrSjio4bXtInrAIoTjVhxcg=="
DEVICE_ID = "midispositivo-dps"   # puede ser cualquier nombre

def derive_device_key(device_id, group_key):
    message = device_id.encode("utf-8")
    signing_key = base64.b64decode(group_key)
    signed_hmac = hmac.new(signing_key, message, hashlib.sha256)
    device_key_encoded = base64.b64encode(signed_hmac.digest())
    return device_key_encoded.decode("utf-8")

print(derive_device_key(DEVICE_ID, GROUP_PRIMARY_KEY))