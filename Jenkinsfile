pipeline {
  agent {
    node {
      label 'node1'
    }

  }
  stages {
    stage('stage1') {
      steps {
        sh 'echo "Hello world!"'
      }
    }

    stage('stage2') {
      steps {
        sh 'echo "Hello world!"'
      }
    }

    stage('') {
      steps {
        sleep 3
      }
    }

  }
}