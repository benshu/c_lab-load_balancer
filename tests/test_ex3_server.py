import unittest
from ex3 import ex3_server
import socket


class ServerTest(unittest.TestCase):
    def test(self):
        self.assertEqual(ex3_server.parse_uri_from_request("GET /foo/bar HTTP/1.0\n"),'/foo/bar')
        self.assertEqual(ex3_server.parse_uri_from_request("GET /foo/bar/ HTTP/1.0\n"),'/foo/bar/')
        self.assertEqual(ex3_server.parse_uri_from_request("GET /foo/bar/HTTP/1.1\n"),'')
        self.assertEqual(ex3_server.handle_request("GET /counter HTTP/1.1\n"),200)
        self.assertEqual(ex3_server.handle_request("GET /counter HTTP/1.1\n"),200)
        self.assertEqual(ex3_server.handle_request("GET /counter1 HTTP/1.1\n"),404)
        self.assertEqual(ex3_server.handle_request("GET /counter HTTP/1.1\n"),200)


if __name__ == "__main__":
    unittest.main()
