<?php
    namespace MyApp;
    use Ratchet\MessageComponentInterface;
    use Ratchet\ConnectionInterface;

    class Chat implements MessageComponentInterface {

        protected $clients;

        public function __construct() {
            $this->clients = new \SplObjectStorage;
        }

        public function onOpen(ConnectionInterface $conn){

            $this->clients->attach($conn);
            echo "New Connection! ({$conn->resourceId})\n";
            $numRecv = count($this->clients) -1;
            echo "Value of $numRecv is " . $numRecv . "\n" ;
            //var_dump($conn->conn); $conn is protected

        }

        public function onMessage(ConnectionInterface $from, $msg){
            $numRecv = count($this->clients) -1;
            
            echo sprintf('Connection %d sending message "%s" to %d other connections%s'. "\n" , $from->resourceId, $msg, $numRecv, $numRecv == 1 ? '' : 's');

            foreach($this->clients as $client){
            
                if($from !== $client){
                    //The sender is not the receiver, send to each client connected
                    $client->send($msg);
                    //$client->close();
                }
            }
            $from->close();

        }

        public function onClose(ConnectionInterface $conn){
            $this->clients->detach($conn);
            echo "Connection {$conn->resourceId} has disconnected\n";

        }

        public function onError(ConnectionInterface $conn, \Exception $e){
            echo "An error has occured: {$e->getMessage()} \n";
            $conn->close();

        }
    
    
    }

?>