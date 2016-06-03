<? 
/* -------------------------------------------------------------------------------------------------- /
/ File		: dbsqlclass.inc
/ Author	: Ganesh S
/ -------------------------------------------------------------------------------------------------- /
/ Description	: 
/     This inc file have DB class to access sql stuff
/ -------------------------------------------------------------------------------------------------- */
class db {
	private $db_host;
	private $db_user;
	private $db_pass;
	private $db_link;
	private $reqStart;
	private $reqEnd;
	public $resource;	
	public $error;
	public $error_code;
	public $error_content;
	public $query;
	
	//constructor
	public function __construct() {
		$this->query = "";
		$this->resetAll();
	}

	//function to reset class variables
	private function resetAll() {
		$this->error = false;
		$this->error_code = "";
		$this->error_content = "";
	}
	
	// create mysql connection //
	public function connect($host, $user, $pass, $dbname='') {
		$this->resetAll();
		$this->db_host = $host;
		$this->db_user = $user;
		$this->db_pass = $pass;
		$link = mysql_connect($this->db_host, $this->db_user, $this->db_pass);
		if(!is_resource($link)) {
			$this->error = true;
			$this->error_code = "DBCONERR";
			$this->error_content = mysql_error();
			$this->logError(mysql_error());
		} else {
			$this->db_link = $link;
			mysql_query("SET NAMES 'utf8'");
		}

		if($dbname!='') {
			$dbsel = mysql_select_db($dbname,$this->db_link);
			if (!$dbsel) {
				$this->error = true;
				$this->error_code = "DBSELERR";
				$this->error_content = mysql_error();
				$this->logError(mysql_error());
			}
		}
	}

	// select query execute method //
	public function select($query) {
		$sub_qry = substr (trim($query),0,6);
		if (strtolower($sub_qry) != 'select' ) {
			$this->error = true;
			$this->error_code = "FNSELERR";
			$this->error_content = mysql_error();
			$this->logError(mysql_error());
		} else {
			$this->query = $query;
			$this->execute();
			$count = mysql_num_rows($this->resource);
			return $count;
		}
	}

	// insert query execute method //
	public function insert($query) {
		$ins_qry = substr (trim($query),0,6);
		if (strtolower($ins_qry) != 'insert' ) {
			$this->error = true;
			$this->error_code = "FNINSERR";
			$this->error_content = mysql_error();
			$this->logError(mysql_error());
		} else {
			$this->query = $query;
			$this->execute();
			return mysql_insert_id($this->db_link);
		}
		
	}

	// update query execute method //
	public function update($query) {
		$up_qry = substr (trim($query),0,6);
		if (strtolower($up_qry) != 'update' ) {
			$this->error = true;
			$this->error_code = "FNUPDERR";
			$this->error_content = mysql_error();
			$this->logError(mysql_error());
		} else {
			$this->query = $query;
			$this->execute();
			$count = mysql_affected_rows($this->db_link);
			return $count;
		}
	}

	// delete query execute method //
	public function del($query) {
		$d_qry = substr (trim($query),0,6);
		if (strtolower($d_qry) != 'delete' ) {
			$this->error = true;
			$this->error_code = "FNDELERR";
			$this->error_content = mysql_error();
			$this->logError(mysql_error());
		} else {
			$this->query = $query;
			$this->execute();
			$count = mysql_affected_rows($this->db_link);
			return $count;
		}
	}

	// query execute method //
	public function execute() {
		$this->resetAll();
		$this->reqStart = time();
		$this->resource = mysql_query($this->query, $this->db_link);
		$this->reqEnd = time();  
		$resTime = ($this->reqEnd - $this->reqStart);
		if($resTime > 6) {
			$this->logSlowQ();
		}
		if(mysql_error()) {
			$this->error = true;
			$this->error_code = "DBQRYERR";
			$this->error_content = mysql_error();
			$this->logError(mysql_error());
		}
	}

	// fetching array result //
	public function fetchArray($type='MYSQL_BOTH',$res='') {
		$resource=($res!="")?$res:$this->resource;
		if ($type == 'MYSQL_ASSOC') {
			$arow = mysql_fetch_array($resource, MYSQL_ASSOC);
		} else if ($type == 'MYSQL_NUM') {
			$arow = mysql_fetch_array($resource, MYSQL_NUM);
		} else {
			$arow = mysql_fetch_array($resource, MYSQL_BOTH);
		}
		return $arow;
	}

	// fetching result as object //
	public function fetchObject() {
		$arow = mysql_fetch_object($this->resource);
		return $arow;
	}

	// get number of rows returned by the select query executed //
	public function getNumRows() {
		return mysql_num_rows($this->resource);
	}

	// get number of rows returned by the select query executed //
	public function getNumFields() {
		return mysql_num_fields($this->resource);
	}

	// get number of rows returned by the insert/update/delete query executed //
	public function getAffectedRows() {
		return mysql_affected_rows($this->db_link);
	}

	//function to scan input for databases
	public function dbEscapeQuotes($value) {
		if(isset($value)) {
			$value = htmlentities($value);
			if (get_magic_quotes_gpc()) {
				$value = stripslashes($value);
			}
			if (!is_numeric($value)) {
				$value = mysql_real_escape_string($value, $this->db_link);
			}
			return $value;	
		}
	}

	// mysql db close method //
	public function dbClose() {
		@mysql_close($this->db_link);
	}

	// free result set //
	public function freeResult() {
		 return (mysql_free_result($this->resource));
	}

	// data seek pointer //
	public function dataSeek($row = '',$res='') {
		$resource=($res)?$res:$this->resource;
		if ($row == '') {
			$row = 0;
		}
		mysql_data_seek ($resource, $row);
		return true;
	}
	
	// to get the debug parameters of the class  //
	public function getDebugParam() {
		$param["host"] = $this->db_host;
		$param["db_link"] = $this->db_link;
		$param["db_resource"] = $this->resource;
		$param["db_error"] = mysql_error();
		return $param;
	}

	// server connection ping //
	public function ping() {
		if (is_resource($this->db_link)) {
			return mysql_ping($this->db_link);
		} else {
			return false;
		}
	}

	// fetch result of query executed in array format //
	public function getResultArray($type='MYSQL_BOTH') {
		if (($type == 'MYSQL_ASSOC') || ($type == 'MYSQL_NUM') || ($type == 'MYSQL_BOTH')) {
			$result_type = $type;
		} else {
			$result_type = "MYSQL_BOTH";
		}
		while($row = $this->fetchArray($result_type)) {
			$result[] = $row;
		}
		return $result;
	}

	// function to insert the array values to the table //
	public function insertArray($table_name, $data) {
			$insert_query = "insert into ";
			$field_list = "";
			$value_list = "";

			if(is_array($data)) {
				foreach($data as $field => $value) {
					if($field_list != "")
						$field_list .= ","; 
					if($value_list != "")
						$value_list .= ",";
					$field_list .= $field;
					$value_list .= mysql_escape_string($value);
				}
			}
			$insert_query .= $table_name."(".$field_list.") values (".$value_list.")";
			$this->query = $insert_query;
			$this->execute();
	}

	// function to update the table with passed array value with where condition //
	public function updateArray($table_name, $data, $where) {
			$update_query = "update ".$table_name." set ";
			$u_query = "";
			if(is_array($data)) {
				foreach($data as $field => $value)
				{
					if($u_query != "")
						$u_query .= ","; 
					$u_query .= $field."=".mysql_escape_string($value);
				}
			}
			$update_query .= $u_query;
			if($where != "")
			{
				$update_query .= "where ".$where;
			}
			$this->query = $update_query;
			$this->execute();
	}

	//write slow query log
	private function logSlowQ() {
		$error = '';
		$error = debug_backtrace();
		$file_list = "";
		$fn_list = "";
		foreach ($error as $row => $error_det) {
			if (($row != 0) && ($row != 1)) {
				$file_list .= $error_det["file"]." > ";
				$fn_list   .= $error_det["function"]." > ";
			}
		}
		$file_content = "\n";
		$file_content .= date('h:i:s').'#';
		$file_content .= ($this->reqEnd - $this->reqStart).'#';
		$file_content .= $this->db_host.'#';
		$file_content .= $this->query.'#';
		$file_content .= $file_list.'#';
		$file_content .= $fn_list.'#';
	}

	// write error log to file //
	private function logError($message) {
		$error = '';
		$error = debug_backtrace();
		$height = count($error);
		$file_list = "";
		$fn_list = "";
		foreach ($error as $row => $error_det) {
			if (($row != 0) && ($row != 1)) {
				$file_list .= $error_det["file"].":~:";
				$fn_list   .= $error_det["function"].":~:";
			}
		}
		$file_content = "\n";
		$file_content .= date('h:i:s').'#~#';
		$file_content .= $this->db_host.'#~#';
		$file_content .= $file_list.'#~#';
		$file_content .= $fn_list.'#~#';
		$file_content .= $this->query.'#~#';
		$file_content .= $message;
	
	}

	function dbConnById ($type='',$value='',$dbtype='',$dbuser='',$dbpass='',$dbname='',$smartsearch='') {
		if ($type=='' || $value=='' || $dbtype=='' || $dbuser=='' || $dbpass=='' || $dbname=='') {
			return "DBCONERR";
		}		
		$this->connect($dbhost, $dbuser, $dbpass, $dbname);
		if ($this->error) {
			return "DBCONERR";
		} elseif ($this->error && $smartsearch=="true") {
			return "DBCONERR";
		}
		return strtoupper($GLOBALS['DOMAINNAME'][$domainid]);
	}

	function chkDbConnTimeOut ($dbip,$port=3306,$timeout=5,$errno=10) {
		$port=3306;
		$timeout=5;
		$errno=10;
		$errstr="Could not connect to database web";
		$testcon = @fsockopen ($dbip, $port, $errno, $errstr, $timeout); 
		if($testcon) {  		 
			$timeoutret = 1;
		} else {
			$timeoutret = 0;
		}
		return $timeoutret;
	}

	function inConnect($host, $user, $pass, $dbname='',$timeout=2){
		$Tresponse = $this->chkDbConnTimeOut ($host,$timeout);
		if($Tresponse){
			$this->connect($host, $user, $pass, $dbname);
			return 1;
		} else {
			return "DBCONERR inconnect";
		}
	}

	// destruct //
	public function __destruct() {
		//@mysql_free_result($this->resource);
		//@mysql_close($this->db_link);
	}
}
?>
