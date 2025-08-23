import os
import berserk

def connect(token: str):
    session = berserk.TokenSession(token)
    return berserk.Client(session=session)
