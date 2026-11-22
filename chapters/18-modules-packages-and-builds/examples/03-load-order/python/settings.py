# The first import of a module runs its top-level code, from top to bottom. Later
# imports find it in sys.modules and don't run it again.
print("settings: loading")
GREETING = "Hello"
