Describe 'Test Test-JobSucceeded.ps1' {

    BeforeAll {
        function testScript ($jobNames) {
            . $PSScriptRoot/Test-JobSucceeded.ps1 `
                -JobNames $jobNames `
                -Project 'testProject' `
                -Org 'testOrg' `
                -BuildId '123' `
                -AccessToken 'testToken'
        }
    }

    It 'Fails on non-200 response' { 
        Mock -CommandName Invoke-RestMethod -MockWith { throw 'Error' }

        { testScript -jobNames TestJob } | Should -Throw
    }

    It 'Fails on invalid response' { 
        Mock -CommandName Invoke-RestMethod -MockWith { @{ invalidObject = "hello" } }

        { testScript -jobNames TestJob } | Should -Throw
    }

    It 'Fails on missing job' {
        Mock -CommandName Invoke-RestMethod -MockWith { @{ records = @(@{ name = "not-TestJob" }) } }

        testScript -jobNames TestJob
        $LASTEXITCODE | Should -Not -Be 0
    }

    It 'Fails on one matching failing job' {
        Mock -CommandName Invoke-RestMethod -MockWith { @{ records = @(@{ name = "TestJob"; result = "failed"; attempt = 1 }) } }

        testScript -jobNames TestJob
        $LASTEXITCODE | Should -Not -Be 0
    }

    It 'Succeeds on one matching succeeded job' {
        Mock -CommandName Invoke-RestMethod -MockWith { @{ records = @(@{ name = "TestJob"; result = "succeeded"; attempt = 1 }) } }

        testScript -jobNames TestJob
        $LASTEXITCODE | Should -Be 0
    }

    It 'Succeeds on two matching jobs with last one succeeding' {
        Mock -CommandName Invoke-RestMethod -MockWith { @{ records = @(@{ name = "TestJob"; result = "failed"; attempt = 1 }, @{ name = "TestJob"; result = "succeeded"; attempt = 2 }) } }

        testScript -jobNames TestJob
        $LASTEXITCODE | Should -Be 0
    }

    It 'Fails on two matching jobs with both failing' {
        Mock -CommandName Invoke-RestMethod -MockWith { @{ records = @(@{ name = "TestJob"; result = "failed"; attempt = 1 }, @{ name = "TestJob"; result = "failed"; attempt = 2 }) } }

        testScript -jobNames TestJob
        $LASTEXITCODE | Should -Not -Be 0
    }
}