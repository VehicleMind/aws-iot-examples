var AWS = require("aws-sdk");
var awsConfig = {
    "region": "ap-south-1",
    "endpoint": "http://dynamodb.ap-south-1.amazonaws.com",
    "accessKeyId": "AKIAJZMYM76N62LM3QDQ", "secretAccessKey": "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"
};
AWS.config.update(awsConfig);

var docClient = new AWS.DynamoDB.DocumentClient();
var remove = function () {

    var params = {
        TableName: "sampletable",
        Key: {
            "name": "Brolin Cheese-Dhoni",
            "city": "pandora"
        }
    };
    docClient.delete(params, function (err, data) {

        if (err) {
            console.log("users::delete::error - " + JSON.stringify(err, null, 2));
        } else {
            console.log("users::delete::success");
        }
    });
}

module.exports.remove = remove
