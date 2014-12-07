from mfs import *

class ExtraCreds(MfsTest):
   name = "extracred"
   description = "write then read one block from block 15"
   timeout = 10

   def run(self):
      self.loadlib()
      self.start_server()

      self.mfs_init("localhost", self.port)
      self.creat(0, MFS_REGULAR_FILE, "test")
      inum = self.lookup(0, "test")

      buf1 = gen_block(1)
      self.write(inum, buf1, 15)

      buf2 = BlockBuffer()
      self.read(inum, buf2, 15)

      if not bufs_equal(buf1, buf2):
         raise Failure("Corrupt data returned by read")

      self.shutdown()

      self.server.wait()
      self.done()

test_list = [ExtraCreds]
